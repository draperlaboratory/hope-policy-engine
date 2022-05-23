import TaggingUtils
import logging
import sys
import yaml

from elftools.elf.constants import SH_FLAGS

def bytes_to_uint(it, size):
    data = []
    for _ in range(size):
        data.append(next(it))
    return int.from_bytes(data, byteorder='little', signed=False)

def round_up(x, align):
    return -((-x) & -align)

class LLVMMetadataTagger:
    PTR_SIZE = 4 # For instructions, not data

    metadata_ops = {
        "DMD_SET_BASE_ADDRESS_OP": 0x01,
        "DMD_TAG_ADDRESS_OP": 0x02,
        "DMD_TAG_ADDRESS_RANGE_OP": 0x03,
        "DMD_TAG_POLICY_SYMBOL": 0x04,
        "DMD_TAG_POLICY_RANGE": 0x05,
        "DMD_TAG_POLICY_SYMBOL_RANKED": 0x06,
        "DMD_TAG_POLICY_RANGE_RANKED": 0x07,
        "DMD_END_BLOCK": 0x08,
        "DMD_END_BLOCK_WEAK_DECL_HACK": 0x09,
        "DMD_FUNCTION_RANGE": 0xa
    }

    # LLVM feature mappings. See header file in LLVM
    tag_specifiers = {
        "DMT_CFI3L_VALID_TGT": 0x01,
        "DMT_STACK_PROLOGUE_AUTHORITY": 0x02,
        "DMT_STACK_EPILOGUE_AUTHORITY": 0x03,
        "DMT_FPTR_STORE_AUTHORITY": 0x04,
        "DMT_BRANCH_VALID_TGT": 0x05,
        "DMT_RET_VALID_TGT": 0x06,
        "DMT_RETURN_INSTR": 0x07,
        "DMT_CALL_INSTR": 0x08,
        "DMT_BRANCH_INSTR": 0x09,
        "DMT_FPTR_CREATE_AUTHORITY": 0x0a,
        "DMT_BEFORE_INLINE_ASM_INSNS": 0x0c,
        "DMT_AFTER_INLINE_ASM_INSNS": 0x0d
    }

    # Names policies will use to access features
    policy_map = {
        "call-tgt": {
            "tag_specifier": tag_specifiers["DMT_CFI3L_VALID_TGT"],
            "name": "llvm.CFI_Call-Tgt"
        },
        "branch-tgt": {
            "tag_specifier": tag_specifiers["DMT_BRANCH_VALID_TGT"],
            "name": "llvm.CFI_Branch-Tgt"
        },
        "return-tgt": {
            "tag_specifier": tag_specifiers["DMT_RET_VALID_TGT"],
            "name": "llvm.CFI_Return-Tgt"
        },
        "call-instr": {
            "tag_specifier": tag_specifiers["DMT_CALL_INSTR"],
            "name": "llvm.CFI_Call-Instr"
        },
        "branch-instr": {
            "tag_specifier": tag_specifiers["DMT_BRANCH_INSTR"],
            "name": "llvm.CFI_Branch-Instr"
        },
        "return-instr": {
            "tag_specifier": tag_specifiers["DMT_RETURN_INSTR"],
            "name": "llvm.CFI_Return-Instr"
        },
        "fptrcreate": {
            "tag_specifier": tag_specifiers["DMT_FPTR_CREATE_AUTHORITY"],
            "name": "llvm.CPI.FPtrCreate"
        },
        "fptrstore": {
            "tag_specifier": tag_specifiers["DMT_FPTR_STORE_AUTHORITY"],
            "name": "llvm.CPI.FPtrStore"
        },
        "prologue": {
            "tag_specifier": tag_specifiers["DMT_STACK_PROLOGUE_AUTHORITY"],
            "name": "llvm.Prologue"
        },
        "epilogue": {
            "tag_specifier": tag_specifiers["DMT_STACK_EPILOGUE_AUTHORITY"],
            "name": "llvm.Epilogue"
        },
        "before_inline_asm": {
            "tag_specifier": tag_specifiers["DMT_BEFORE_INLINE_ASM_INSNS"],
            "name": "llvm.Inline"
        },
        "after_inline_asm": {
            "tag_specifier": tag_specifiers["DMT_AFTER_INLINE_ASM_INSNS"],
            "name": "llvm.Inline"
        }
    }
    latest_inline_addr = -1
    latest_inline_addr_valid = False

    def __init__(self):
        self.needs_tag_cache = {}

    def policy_needs_tag(self, policy_inits, tag):

        if tag in self.needs_tag_cache:
            return self.needs_tag_cache[tag]

        d = policy_inits['Require']
        for item in tag.split("."):
            try:
                d = d[item]
            except KeyError:
                print("Don't need tag " + tag)
                self.needs_tag_cache[tag] = False
                return False

        self.needs_tag_cache[tag] = True
        print("Yes need tag " + tag)
        return True

    def add_code_section_ranges(self, elf_file, range_map):
        for s in elf_file.iter_sections():
            flags = s['sh_flags']
            if (flags & SH_FLAGS.SHF_ALLOC):
                start = s['sh_addr']
                end = start + s['sh_size']
                end = round_up(end, self.PTR_SIZE)
                if ((flags & (SH_FLAGS.SHF_ALLOC | SH_FLAGS.SHF_WRITE | SH_FLAGS.SHF_EXECINSTR)) ==
                    (SH_FLAGS.SHF_ALLOC | SH_FLAGS.SHF_EXECINSTR)):
                    range_map.add_range(start, end)

    def check_and_write_range(self, range_file, start, end, tag_specifier,
                            policy_inits, range_map):
        for policy, tags in self.policy_map.items():
            if self.policy_needs_tag(policy_inits, tags['name']) or self.policy_needs_tag(policy_inits, "llvm.Inline"):
                if tags['tag_specifier'] == tag_specifier:
#                     print(policy + ": start " + hex(start) + ", end " + hex(end));
                    if tags['name'] == "llvm.Prologue":
                        self.latest_inline_addr_valid = False
                        self.latest_inline_addr = start + 4;
                    if tags['name'] == "llvm.Epilogue" and self.latest_inline_addr > 0 and self.latest_inline_addr_valid:
                            print("Should tag between " + hex(self.latest_inline_addr) + " and epilogue at " + hex(start) + " as INLINE_ASM")
                            range_file.write_range(self.latest_inline_addr, start, "llvm.Inline")
                            range_map.add_range(self.latest_inline_addr, start, "llvm.Inline")
                            self.latest_inline_addr = -1
                            self.latest_inline_addr_valid = False

                    if tags['name'] == "llvm.Inline":
                        if tag_specifier == self.tag_specifiers["DMT_BEFORE_INLINE_ASM_INSNS"]:
                            self.latest_inline_addr_valid = True
                            self.latest_inline_addr = start + 4
                        elif self.latest_inline_addr > 0 and tag_specifier == self.tag_specifiers["DMT_AFTER_INLINE_ASM_INSNS"]:
                            # Do something
                            if self.latest_inline_addr_valid:
                                print("Should tag between " + hex(self.latest_inline_addr) + " and " + hex(start) + " as INLINE_ASM")
                            else:
                                print("Should tag between prologue at " + hex(self.latest_inline_addr) + " and " + hex(start) + " as INLINE_ASM")
                            range_file.write_range(self.latest_inline_addr, start, tags['name'])
                            range_map.add_range(self.latest_inline_addr, start, tags['name'])
                            self.latest_inline_addr = -1
                            self.latest_inline_addr_valid = False
                            pass
                        elif tag_specifier == self.tag_specifiers["DMT_BEFORE_INLINE_ASM_INSNS"]:
                            print("Invalid double before tag " + hex(self.latest_inline_addr) + " and " + hex(start) + ".")
                            # Maybe if we hit prologue or epilogue, we should reset vakyes, too.
                        else:
                            print("Invalid after inline tag with no before " + hex(start) + ".")
                    elif self.policy_needs_tag(policy_inits, tags['name']):
                        range_file.write_range(start, end, tags['name'])
                        range_map.add_range(start, end, tags['name'])

    def generate_policy_ranges(self, elf_file, range_file, policy_inits):
        metadata = elf_file.get_section_by_name(b'.dover_metadata')
        if not metadata:
            metadata = elf_file.get_section_by_name(".dover_metadata")
        assert metadata, "No metadata found in ELF file!"
        metadata = metadata.data()
        assert metadata[0] == self.metadata_ops['DMD_SET_BASE_ADDRESS_OP'], "Invalid metadata found in ELF file!"

        it = iter(metadata)

        range_map = TaggingUtils.RangeMap()

        for byte in it:
            if (byte == self.metadata_ops['DMD_SET_BASE_ADDRESS_OP']):
                base_address = bytes_to_uint(it, 8) #apparently GCC emits a 64-bit base
                logging.debug("new base address is " + hex(base_address) + "\n")
            elif (byte == self.metadata_ops['DMD_TAG_ADDRESS_OP']):
                address = bytes_to_uint(it, self.PTR_SIZE) + base_address
                tag_specifier = bytes_to_uint(it, 1)
                logging.debug("tag is " + hex(tag_specifier) +
                            " at address " + hex(address) + '\n')

                self.check_and_write_range(range_file, address, address + self.PTR_SIZE,
                                    tag_specifier, policy_inits, range_map)

            elif (byte == self.metadata_ops['DMD_TAG_ADDRESS_RANGE_OP']):
                start_address = bytes_to_uint(it, self.PTR_SIZE) + base_address
                end_address = bytes_to_uint(it, self.PTR_SIZE) + base_address
                tag_specifier = bytes_to_uint(it, 1)
                logging.debug("tag is " + hex(tag_specifier) +
                            " for address range " +
                            hex(start_address) + ":" + hex(end_address) + '\n')

                self.check_and_write_range(range_file, start_address, end_address,
                                    tag_specifier, policy_inits, range_map)

            elif (byte == self.metadata_ops['DMD_TAG_POLICY_SYMBOL']):
                logging.critical("Saw policy symbol!\n")
                sys.exit(-1)
            elif (byte == self.metadata_ops['DMD_TAG_POLICY_RANGE']):
                logging.critical("Saw policy range!\n")
                sys.exit(-1)
                for _ in range(self.PTR_SIZE*3):
                    #skip start, end, & 32-bit tag-type
                    next(it)
            elif (byte == self.metadata_ops['DMD_TAG_POLICY_SYMBOL_RANKED']):
                logging.critical("Saw policy symbol ranked\n")
                sys.exit(-1)
            elif (byte == self.metadata_ops['DMD_TAG_POLICY_RANGE_RANKED']):
                sys.exit(-1)
                logging.critical("Saw policy symbol range ranked\n")
                #skip start, end ,tag category, rank, tag type
                for _ in range(self.PTR_SIZE*5):
                    next(it)
            elif (byte == self.metadata_ops['DMD_END_BLOCK_WEAK_DECL_HACK']):
                logging.critical("saw end weak decl tag!\n")
                sys.exit(-1)
            elif (byte == self.metadata_ops['DMD_END_BLOCK']):
                end_address = bytes_to_uint(it, self.PTR_SIZE)
                logging.debug("saw end block tag range = " + hex(base_address) +
                            ":" + hex(base_address + end_address))
                range_map.add_range(base_address, base_address + end_address, "COMPILER_GENERATED")
            elif (byte == self.metadata_ops['DMD_FUNCTION_RANGE']):
                start_address = bytes_to_uint(it, self.PTR_SIZE) + base_address
                end_address = bytes_to_uint(it, self.PTR_SIZE) + base_address
                logging.debug("saw function range = " + hex(start_address) +
                            ":" + hex(end_address))
                range_map.add_range(start_address, end_address, "COMPILER_GENERATED")
            else:
                logging.debug("Error: found unknown byte in metadata!" + hex(byte) + "\n")
                sys.exit(-1)

        # tag NoCFI for anything not specifically noted by llvm
        if 'NoCFI' in policy_inits['Require']['llvm']:

            code_range_map = TaggingUtils.RangeMap()
            self.add_code_section_ranges(elf_file, code_range_map)

            for (start, end, tags) in code_range_map:
                for s in range(start, end, self.PTR_SIZE):
                    e = s + self.PTR_SIZE
                    if (s, e, tags) not in range_map:
                        range_file.write_range(s, e, "llvm.NoCFI")

        return range_map
