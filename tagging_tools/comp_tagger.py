# This is the compartment tagger to support compartmentalization policies.
#
# add_function_ranges() assigns a unique identifier to each function in a program.
# add_object_ranges() assigns a unique identifier to each object, including globals,
# heap allocation sites, and a few special kinds of objects.
#
# Both functions add their output into a range_file, which gets consumed by md_range
# to get incorporated into the .taginfo file.
# The tags placed on these words are either "Comp.funcID" or "Comp.globalID".
# The actual identifier is added to the .taginfo.args file which eventually
# gets set on the field values.
#
# These functions also produce func_defs.h and object_defs.h, which provide
# string labels for the created object identifiers for pretty printing / analysis.
# Note that currently add_object_ranges() must be called first, so that the heap_id
# can be passed to the function range tagger. TODO some cleanup. Some additional
# functionality got added here, such as context-switching. Also note that there
# is a lot of architecture dependence (uses riscv nm and objdump to extract
# some required information).

import sys
import shutil
import os.path
import subprocess
from elftools.elf.elffile import ELFFile
from elftools.elf.constants import SH_FLAGS
from elftools.common.py3compat import itervalues
from elftools.dwarf.locationlists import LocationEntry
from elftools.dwarf.descriptions import describe_form_class

# Function to label each function in a program with a unique tag.
# Puts the Comp.funcID tag on each instruction in the program, then writes
# a unique identifier for that function in the taginfo.args file.
# Also generates a func_defs.h header with labels for the identifiers.
def add_function_ranges(elf_filename, range_file, taginfo_args_file, policy_dir, heap_id_start, obj_map, subject_map):

    # Create new defs file for function labels
    defs_file = open("func_defs.h", "w")
    defs_file.write("const char * func_defs[] = {\"<none>\",\n")

    # If we have a subject map, load it in and print it out into defs file
    if subject_map != None:
        has_subject_map = True
        print("Subject map loaded!")
        subj_map_inverted = {}
        for subj in subject_map:
            if subj[-8:] != "_cluster":
                num = subject_map[subj]
                subj_map_inverted[num] = subject_map[subj + "_cluster"]
        min_subj = min(subj_map_inverted.keys())
        max_subj = max(subj_map_inverted.keys())
        print("Subjects in map range from " + str(min_subj) + " to " + str(max_subj))
        for i in range(min_subj, max_subj + 1):
            defs_file.write("\"" + subj_map_inverted[i] + "\",\n")
        last_function_number = max_subj
    else:
        last_function_number = 0
        has_subject_map = False
        print("NOTE: no subject definition file found, using per-function subjects.")

    # Get all the malloc() sites, we also put a unique label on each malloc site
    # in this function. TODO could be separate pass.
    mallocs = extract_malloc_sites(elf_filename, policy_dir, heap_id_start, obj_map)

    # Keep a set of all code locations that we tag.
    # This is used to clean up some edge cases later and to make sure
    # that all executable instructions end up with *some* identifier.
    tagged_instructions = set()
    
    # Open ELF
    with open(elf_filename, 'rb') as elf_file:

        ef = ELFFile(elf_file)

        # See if we have DWARF info. Currently required
        if not ef.has_dwarf_info():
            raise Exception('  file has no DWARF info')
            return

        dwarfinfo = ef.get_dwarf_info()

        function_numbers = {}

        # Code tagging pass 1:
        # Iterate through each compilation unit, take each function that has
        # a DW_TAG_subprogram entry, and give proper label.
        # Some code below taken from decode_funcname() in the ELFtools examples.
        print("Beginning pass 1")
        for CU in dwarfinfo.iter_CUs():
            for DIE in CU.iter_DIEs():
                try:
                    # We only care about functions (subprograms)
                    if str(DIE.tag) == "DW_TAG_subprogram":

                        if not ("DW_AT_name" in DIE.attributes and "DW_AT_low_pc" in DIE.attributes):
                            #print("Skipping a subprogram DIE.")
                            continue

                        
                        func_name = DIE.attributes["DW_AT_name"].value.decode("utf-8")
                        func_display_name = str(func_name)
                        print("Compartment tagger: tagging function " + func_display_name)

                        # Get subject_id for this function. If we have a provided domain map, look it up there.
                        # Otherwise, create a fresh one.
                        if has_subject_map:                            
                            if not func_display_name in subject_map:
                                raise Exception("Provided subject domain mapping did not include function " + func_display_name)
                            subject_id = subject_map[func_display_name]
                        else:
                            if func_name in function_numbers:
                                function_number = function_numbers[func_name]
                            else:
                                last_function_number += 1
                                function_number = last_function_number
                                function_numbers[func_name] = function_number
                                subject_id = function_number
                                # Create new line in defs file for this function
                                defs_file.write("\"" + func_display_name + "\",\n")
                        
                        low_pc = DIE.attributes['DW_AT_low_pc'].value

                        # DWARF v4 in section 2.17 describes how to interpret the
                        # DW_AT_high_pc attribute based on the class of its form.
                        # For class 'address' it's taken as an absolute address
                        # (similarly to DW_AT_low_pc); for class 'constant', it's
                        # an offset from DW_AT_low_pc.
                        high_pc_attr = DIE.attributes['DW_AT_high_pc']
                        high_pc_attr_class = describe_form_class(high_pc_attr.form)
                        if high_pc_attr_class == 'address':
                            high_pc = high_pc_attr.value
                        elif high_pc_attr_class == 'constant':
                            high_pc = low_pc + high_pc_attr.value
                        else:
                            print('Error: invalid DW_AT_high_pc class:',
                                  high_pc_attr_class)
                            continue

                        # The high address given here is first address *not* part of the func
                        # We now have the low addr, high addr, and name.
                        print("Tagging from " + hex(low_pc) + " " + hex(high_pc))
                        
                        # Add Comp.funcID to taginfo file
                        range_file.write_range(low_pc, high_pc, "Comp.funcID")

                        # Add each of these addresses into our set of known instructions
                        for addr in range(low_pc, high_pc, 4):
                            tagged_instructions.add(addr)

                            
                        # Then make another pass, add entries for any malloc() call sites within that range
                        for addr in range(low_pc, high_pc, 4):
                            if addr in mallocs:
                                alloc_num = mallocs[addr]
                                print("Tagging instr " + hex(addr) + " with alloc-num " + str(alloc_num))
                                #print("There was an allocation call site within this func: " + str(addr))
                                taginfo_args_file.write('%x %x %s\n' % (addr, addr + 4, str(subject_id) + " 0 " + str(alloc_num)))

                        # Set the field for these instruction words in the taginfo_args file.
                        taginfo_args_file.write('%x %x %s\n' % (low_pc, high_pc, str(subject_id) + " 0 0"))
                                                
                except KeyError:
                    print("KeyError: " + str(KeyError))
                    continue

        # Code tagging pass 2:
        # Some code will not have a DW_TAG_subprogram entry, such as code that came from
        # a handwritten assembly source file. In this pass we take code that is not yet labeled,
        # and give it a label based on the compilation unit it came from.
        # If that code belongs to a function that is in our mapping, we'll give it that tag instead
        # of the general CU tag.
        print("Beginning pass 2")        
        for CU in dwarfinfo.iter_CUs():
            for DIE in CU.iter_DIEs():
                if str(DIE.tag) == "DW_TAG_compile_unit":

                    # Check to see if has both low_pc and high_pc
                    if "DW_AT_low_pc" in DIE.attributes and "DW_AT_high_pc" in DIE.attributes:
                        
                        # Extract low_pc
                        low_pc = DIE.attributes["DW_AT_low_pc"].value

                        # Extract high_pc. Might be encoded as length or addr
                        highpc_attr = DIE.attributes['DW_AT_high_pc']
                        highpc_attr_class = describe_form_class(highpc_attr.form)
                        if highpc_attr_class == 'address':
                            high_pc = highpc_attr.value
                        elif highpc_attr_class == 'constant':
                            high_pc = low_pc + highpc_attr.value

                        # Get name
                        cu_src = DIE.attributes["DW_AT_name"].value.decode("utf-8")
                            
                        # If *every* instruction tagged, then nothing came from a .S and we can skip
                        skip = True
                        for addr in range(low_pc, high_pc, 4):
                            if not addr in tagged_instructions:
                                skip = False
                        if skip:
                            print("CU that was fully tagged! " + hex(low_pc) + "->" + hex(high_pc) + " Skipping " + cu_src)
                            continue
                        else:
                            print("CU had some gaps. Now applying to " + cu_src + " over range" + hex(low_pc) + "->" + hex(high_pc))
                            pass

                        # Add function to range, mark ID in arg file
                        range_file.write_range(low_pc, high_pc, "Comp.funcID")
                        
                        # Create a new function name and def entry for this subject
                        function_name = "CU_" + os.path.basename(cu_src)
                        print("Compartment tagger: tagging function " + function_name)

                        #print("There was at least one untagged instruction in cu " + cu_src)
                        
                        # If it's from portASM, add special 'context-switch' tag over it too
                        # TODO: currently leaving out the context-switching logic, maybe add back
                        # and do more debugging
                        
                        if "portASM" in function_name:
                            print("Adding context-switch from " + hex(low_pc) + " to " + str(high_pc))
                            range_file.write_range(low_pc, high_pc, "Comp.context-switch")
                        

                        # If we have a subject map, use that. Otherwise, use a per-CU tag.
                        # This means subject map may need to include regions of assembly (CU_start.S) to pass.
                        if has_subject_map:
                            if not function_name in subject_map:
                                raise Exception("Provided subject domain mapping did not include function " + function_name)
                            subject_id = subject_map[function_name]                            
                        else:
                            function_number += 1                            
                            defs_file.write("\"" + function_name + "\",\n")
                            subject_id = function_number
                            #taginfo_args_file.write('%x %x %s\n' % (low_pc, high_pc, str(function_number) + " 0 0"))

                        #print("\tGot cluster for this func="+str(subject_id))
                        #print("Writing CU over gaps between %x and %x\n" % (low_pc, high_pc))
                        current_pc = low_pc
                        untagged_start = None
                        while current_pc < high_pc:

                            # Detect beginning of new untagged block we need to hit
                            if not current_pc in tagged_instructions and untagged_start == None:
                                untagged_start = current_pc
                                print("New untagged starting " + hex(untagged_start))

                            # Detect end of current untagged block, tag it
                            if untagged_start != None and current_pc in tagged_instructions:
                                taginfo_args_file.write('%x %x %s\n' % (untagged_start, current_pc, str(subject_id) + " 0 0"))
                                print("Cut a range of untagged: " + hex(untagged_start) + " " + hex(current_pc))
                                untagged_start = None

                            current_pc += 4

                        # Lastly, if we exit loop without cleaning up last block, then tag it
                        if untagged_start != None:
                            taginfo_args_file.write('%x %x %s\n' % (untagged_start, high_pc, str(subject_id) + " 0 0"))

                        # Track that these instructions are now labeled
                        for addr in range(low_pc, high_pc, 4):
                            if not addr in tagged_instructions:
                                tagged_instructions.add(addr)

                    elif "DW_AT_low_pc" in DIE.attributes:
                        low_pc = DIE.attributes["DW_AT_low_pc"].value
                        
                else:
                    pass
                
        # Lastly, just print out any executable memory that didn't get a tag.
        # I've seen some padding bytes at end of .text section flagged by this.
        # This is for debugging / sanity checking reasons
        for s in ef.iter_sections():
            flags = s['sh_flags']
            start = s['sh_addr']
            end = start + s['sh_size']
            added_from_section = False
            function_name = None
            if flags & SH_FLAGS.SHF_EXECINSTR:
                for addr in range(start, end, 4):
                    if not addr in tagged_instructions:
                        print("Potential warning: found not-tagged executable instr " + hex(addr) + " in section " + s.name)
        
    # Finish off definition file, then copy into policy include folder
    defs_file.write("\"\"\n};\n")
    defs_file.close()
    shutil.copy("func_defs.h", os.path.join(policy_dir, "include"))
    print("Done tagging functions.")


# Using the environment variable COMP_DOMAINS look into the domains
# subdirectory of policy kernel to see if we have a match for the selected
# domains and binary name. This allows us to easily run various subject domains.
def find_domains(policy_dir, elf_filename):

    if "COMP_DOMAINS" in os.environ:
        domains = os.environ['COMP_DOMAINS']

        # Default is functions, but there is no file for this reflexive case.
        # So we just interpret the string "func" as same as being unset
        if domains == "":
            print("Empty domains! Using default (funcs)")
            return (None, None)
    else:
        print("Empty domains! Using default (funcs)")
        return (None, None)
    
    print("Compartmentalization policy, selected domains: " + domains)

    domain_dir = os.path.join(policy_dir, "domains")
    binary_name = os.path.basename(elf_filename)
    domain_filename = os.path.join(domain_dir, binary_name + "." + domains + ".domains")
    (subject_map, object_map) = load_domains(domain_filename)
    return (subject_map, object_map)

# Load in a subject definition file, return as a dict.
# Definition file is a simple mapping of function name
# or object name to an ID. 
# Subj Foo1 1
# Subj Foo2 1
# Subj Foo3 2
# Obj Obj1 1
# Means Foo1 and Foo2 are grouped together, Foo3 is alone.
def load_domains(filename):
    
    fh = open(filename, "r")
    subject_map = {}
    object_map = {}
    
    for line in fh:
        parts = line.split()
        if len(parts) != 4:
            raise Exception("Format error of subject defs file. Expected four tokens.")
        entry_type = parts[0]
        name = parts[1]
        cluster_id = int(parts[2])
        cluster_name = parts[3]
        if entry_type == "Subj":
            subject_map[name] = cluster_id
            subject_map[name + "_cluster"] = cluster_name
        elif entry_type == "Obj":
            object_map[name] = cluster_id
            object_map[name + "_cluster"] = cluster_name
        else:
            raise Exception("Error reading domain file, invalid entry type.")

    print("Read " + str(len(subject_map)) + " subj mapping and " +
          str(len(object_map)) + " object mappings.")
    
    return (subject_map, object_map)
        
# Analog to add_function_ranges() for extracting objects.  I tried to
# get globals via pyelftools, but it doesn't look like the size/type
# DIEs are parsed for some reason. I spent a few hours and couldn't
# figure it out. Temp solution is to just dump from nm.
# TODO: this is currently just dumping global info from nm, should get from compiler metadata
def add_object_ranges(elf_filename, range_file, taginfo_args_file, policy_dir, object_map):
    
    # Check to make sure we have the tools we need for object extraction
    isp_prefix = os.environ['ISP_PREFIX']
    nm = os.path.join(isp_prefix, "riscv32-unknown-elf", "bin", "nm")

    if not os.path.isfile(nm):
        raise Exception("WARNING: could not find nm. Looked for " + nm)

    # Begin object numbering.
    # 0 = the "none" object, should get no references
    # 1 = Special-IO, a special tag that goes on all mapped IO devices
    # 2 = Special-RAM, a special tag on all RAM that didn't get labeled as something else
    # 3 = Special-FLASH, a special tag for Flash mem that didn't get labeled as something else
    # 4 = Special-UART, a special tag for UART mem
    # 5 = Special-PLIC, a special tag for PLIC mem
    # 6 = Special-ETHERNET, a special tag for ETHERNET

    # Create file for object definition labels
    defs_file = open("object_defs.h", "w")
    defs_file.write("const char * object_defs[] = {\"<none>\",\"special-IO\",\n\"special-RAM\",\n\"special-FLASH\",\n\"special-UART\",\n\"special-PLIC\",\n\"special-ETHERNET\",\n")    

    # Start object_number from 6 or 7 depending on how we loaded (TODO think through)

    # If we have an object map, print them out now in order. Invert then go numerically.
    if object_map != None:
        object_number = 7        
        # Invert the map
        object_map_inverted = {}
        for obj in object_map:
            if obj[-8:] != "_cluster":
                num = object_map[obj]
                object_map_inverted[num] = object_map[obj + "_cluster"]
        min_obj = min(object_map_inverted.keys())
        max_obj = max(object_map_inverted.keys())
        print("Objects in map range from " + str(min_obj) + " to " + str(max_obj))
        for i in range(min_obj, max_obj + 1):
            defs_file.write("\"" + object_map_inverted[i] + "\",\n")
            object_number += 1
    else:
        object_number = 6

    # Keep track of which addresses we've labeled
    tagged_addrs = {}    

    # First pass, we use nm to create objects for all global variables
    p = subprocess.Popen([nm, "-S", elf_filename], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # At the end of the nm pass, do a sorted pretty print of all the globals we found
    sorted_globals = []
    
    while True:
        line = p.stdout.readline().decode("utf-8").strip()
        parts = line.split()
        if len(parts) == 4:

            # Decode format of nm output
            addr = parts[0]
            size = parts[1]
            code = parts[2]
            name = parts[3]
            obj_name = "global_" + name

            # Grab globals. Assuming come from .bss, .data, .ro...
            if code in ["b", "B", "d", "D", "r", "R", "g", "G"]:

                # Create identifier for this object
                if object_map != None:
                    if obj_name in object_map:
                        object_id = object_map[obj_name]
                    else:
                        object_number += 1
                        object_id = object_number
                        print("Could not find object mapping for " + obj_name + ", assigned to " + str(object_id))
                        defs_file.write("\"" + "global_" + name + "\",\n")                        
                else:
                    defs_file.write("\"" + "global_" + name + "\",\n")
                    object_number += 1
                    object_id = object_number
                
                print("Compartment tagger: tagging global " + name + " at address " + addr + " size=" + size + " with ID " + str(object_id))
                sorted_globals.append((addr, size, name))

                # Compute highpc, needed for range file format
                lowpc = int(addr, 16)
                size = int(size, 16)

                # Current compiler does not place each logical object (global variable)
                # in its own word. As a result, odd sized objects (3 bytes, etc) can end
                # up sharing a word with other objects. For now, just rounding each obj
                # to nearest word and letting one object win. In the fure, compiler should
                # pad objects out to full words to eliminate this problem.
                
                # Round lowpc down to align it
                while lowpc % 4 != 0:
                    lowpc -=1
                    
                highpc = lowpc + size

                # Round up high-pc to end of word
                while highpc % 4 != 0:
                    highpc += 1

                # First, mark in our map which addrs this will hit
                for addr in range(lowpc, highpc, 4):
                    if not addr in tagged_addrs:
                        tagged_addrs[addr] = name
                    else:
                        print("Warning: overlap of " + name + " and " + tagged_addrs[addr])
                        highpc = addr - 4
                        break

                # Sanity check
                if highpc - lowpc == 0:
                    print("Size = 0, skipping.")
                    continue
                
                range_file.write_range(lowpc, highpc, "Comp.globalID")
                taginfo_args_file.write('%x %x %s\n' % (lowpc, highpc, str(object_id) + " 0 0"))
                
        # Exit when no more output from nm
        if not line:
            break

    # Print a sorted list of found globals
    '''
    sorted_globals = sorted(sorted_globals)
    for (addr, size, name) in sorted_globals:
        print(addr + " " + size + " " + name)
    '''

    # Next, create a few special objects from ELF sections
    # These include the system stack, and adding anon ids
    # to all words inside global regions that didn't get labels
    # from nm pass.
    with open(elf_filename, 'rb') as elf_file:

        ef = ELFFile(elf_file)

        for s in ef.iter_sections():
            flags = s['sh_flags']
            start = s['sh_addr']
            end = start + s['sh_size']

            # Add the special STACK tag over the .stack section
            if s.name == ".stack":
                object_number += 1
                range_file.write_range(start, end, "Comp.globalID")
                taginfo_args_file.write('%x %x %s\n' % (start, end, str(object_number) + " 0 0"))
                defs_file.write("\"" + "STACK" + "\",\n")

            # Every word inside .rodata, .data or .bss that we don't know about gets an unknown tag
            if s.name in [".rodata", ".data", ".bss"]:
                object_number += 1
                defs_file.write("\"" + s.name + "_UNKNOWN" + "\",\n")
                for addr in range(start, end, 4):
                    if addr not in tagged_addrs:
                        print("Unlabeled data at " + hex(addr) + " from sect " + s.name)
                        range_file.write_range(addr, addr + 4, "Comp.globalID")
                        taginfo_args_file.write('%x %x %s\n' % (addr, addr + 4, str(object_number) + " 0 0"))

            print("Section " + s.name + " from " + hex(start) + " to " + hex(end))

    print("Added " + str(object_number) + " globals + special objects.")
    
    return object_number

# This function extracts all the allocation sites from a program and
# gives each one a unique identifier.
# Current implementation is quite cheesy, it just uses objdump and
# the keyword "pvPortMalloc", which is only suitable for FreeRTOS.
# TODO generalize, get from LLVM, etc
def extract_malloc_sites(elf_filename, policy_dir, heap_id_start, obj_map):
    
    # Check for objdump:
    isp_prefix = os.environ['ISP_PREFIX']
    objdump = os.path.join(isp_prefix, "riscv32-unknown-elf", "bin", "objdump")

    if not os.path.isfile(objdump):
        raise Exception("WARNING: could not find objdump. Looked for " + objdump)

    defs_file = open("object_defs.h", "a")

    p = subprocess.Popen([objdump, "-d", elf_filename], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    mallocs = {}
    
    # Process objdump output
    current_func = ""
    object_id = heap_id_start
    mallocs_in_func = 0
    heap_obj_basename = ""
    for line in p.stdout:
        line = line.decode("utf-8").strip()
        parts = line.split()


        # Track current function and how many allocs in this function
        if ">:" in line and not ".LB" in line:
            current_func = parts[1][1:-2]
            mallocs_in_func = 0
            heap_obj_basename = "heap_" + current_func
            #print("Set heap_obj_basename to " + heap_obj_basename)

        # Each static call to pvPortMalloc() is a unique allocation
        if "<pvPortMalloc>" in line and "jal" in line:
            call_addr_string = parts[0][:-1]
            call_addr = int(call_addr_string, 16)
            mallocs_in_func += 1

            if mallocs_in_func > 1:
                heap_lookup = heap_obj_basename + str(mallocs_in_func)
                heap_label = "heap_" + current_func + str(mallocs_in_func)
            else:
                heap_lookup = heap_obj_basename
                heap_label = "heap_" + current_func
                
            if obj_map != None and heap_lookup in obj_map:
                object_id = obj_map[heap_lookup]
                print("Found heap obj in lookup! Assigned to " + str(object_id))
                mallocs[call_addr] = object_id
            else:
                print("Could not find heap obj in lookup. Using fallback ID.")
                object_id += 1
                mallocs[call_addr] = object_id
                defs_file.write("\"" + heap_label + "\",\n")
                
            print("Heap obj " + str(object_id) + " going on line " + line)                
            
                

    # Finish off definition file, then copy into policy include folder
    #defs_file.write("\"TESTOBJ\",\n")
    defs_file.write("\"\"};\n")
    defs_file.close()
    print("policy_dir: " + policy_dir)
    shutil.copy("object_defs.h", os.path.join(policy_dir, "include"))    

    # Return dictionary of found mallocs
    print("Got a total of " + str(len(mallocs)) + " allocation site objects.")
    return mallocs


def rebuildCompPolicy(arch):

    # 1) Copy fresh policy into isp/validator/policy
    print("Rebuilding the comp policy with new subjs/objs...")
    isp_prefix = os.environ['ISP_PREFIX']
    policy_dir = os.path.join(isp_prefix, "policies", "compartmentalization")
    validator_dir = os.path.join(isp_prefix, "validator")
    engine_dir = os.path.join(validator_dir, "isp-install-compartmentalization", "engine")
    validator_policy_dir = os.path.join(engine_dir, "policy")

    if not os.path.exists(policy_dir):
        print("ERROR: could not find policy dir: " + policy_dir)
    if not os.path.exists(validator_dir):
        print("ERROR: could not find validator dir: " + validator_policy_dir)

    shutil.rmtree(validator_policy_dir)
    shutil.copytree(policy_dir, validator_policy_dir)

    # 2) Rebuild policy
    #cur_dir = os.getcwd()
    #os.chdir(validator_dir)
    build_log = open("rebuild.out", "w+")
    build_log.write("Rebuilding...\n");
    print("Rebuilding...")
    result = subprocess.call(["make", "-f", "Makefile.isp"], stdout=build_log, stderr=subprocess.STDOUT, cwd=engine_dir)

    # 3) Move validator
    validator_path = os.path.join(engine_dir, "build", "lib{}-sim-validator.so".format(arch))

    try:
        validator_out_name = arch + "-" + "compartmentalization" + "-validator.so"
        validator_out_path = os.path.join(validator_dir, validator_out_name)
        print("Trying to move " + validator_path + " to " + validator_out_path)
        shutil.move(validator_path, validator_out_path)
        #shutil.rmtree(output_dir)
        
    except Exception as e:
        logger.error("Moving validator to output dir failed with error: {}".format(e))
        return False
    
    print("Done rebuilding comp validator!")
