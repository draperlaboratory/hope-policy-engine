import TaggingUtils
import logging
import sys
import yaml

PTR_SIZE = 4

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
    "DMT_FPTR_CREATE_AUTHORITY": 0x0a
}

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
        "name": "dover.Tools.GCC.Prologue"
    },
    "epilogue": {
        "tag_specifier": tag_specifiers["DMT_STACK_EPILOGUE_AUTHORITY"],
        "name": "dover.Tools.GCC.Epilogue"
    }
}


def bytes_to_uint(it, size):
    data = []
    for _ in range(size):
        data.append(next(it))
    return int.from_bytes(data, byteorder='little', signed=False)


def policy_needs_tag(tag):

    d = policy_inits_yaml['Require']
    for item in tag.split("."):
        try:
            d = d[item]
        except KeyError:
            return False

    return True

def check_and_write_range(range_file, start, end, tag_specifier,
                          policy, range_map=None):
    for policy, tags in policy_map.items():
        if policy_needs_tag(tags['name']):
            if tags['tag_specifier'] == tag_specifier:
                range_file.write_range(start, end, tags['name'])
                if range_map:
                    range_map.add_range(start, end, tags['name'])


def generate_policy_ranges(ef, range_file, policy):
    metadata = ef.get_section_by_name(b'.dover_metadata')
    if not metadata:
        metadata = ef.get_section_by_name(".dover_metadata")
    metadata = metadata.data()
    assert metadata[0] == metadata_ops['DMD_SET_BASE_ADDRESS_OP'], "Invalid metadata found in ELF file!"

    it = iter(metadata)

    global policy_inits_yaml

    with open("/home/scott/hope/policies/policy_tests/output/webapp.hifive/osv.hifive.main.ppac-usr_type/osv.hifive.main.ppac-usr_type/policy_init.yml", "r") as pmf:
        policy_inits_yaml = yaml.load(pmf.read())    
    
    if policy == "cfi" or policy == "threeClass" or policy == "ppac":
        range_map = TaggingUtils.RangeMap()
    else:
        range_map = None

    for byte in it:
        if (byte == metadata_ops['DMD_SET_BASE_ADDRESS_OP']):
            base_address = bytes_to_uint(it, 8) #apparently GCC emits a 64-bit base
            logging.debug("new base address is " + hex(base_address) + "\n")
        elif (byte == metadata_ops['DMD_TAG_ADDRESS_OP']):
            address = bytes_to_uint(it, PTR_SIZE) + base_address
            tag_specifier = bytes_to_uint(it, 1)
            logging.debug("tag is " + hex(tag_specifier) +
                          " at address " + hex(address) + '\n')

            check_and_write_range(range_file, address, address + PTR_SIZE, tag_specifier,
                                  policy, range_map)

        elif (byte == metadata_ops['DMD_TAG_ADDRESS_RANGE_OP']):
            start_address = bytes_to_uint(it, PTR_SIZE) + base_address
            end_address = bytes_to_uint(it, PTR_SIZE) + base_address
            tag_specifier = bytes_to_uint(it, 1)
            logging.debug("tag is " + hex(tag_specifier) +
                          " for address range " +
                          hex(start_address) + ":" + hex(end_address) + '\n')

            check_and_write_range(range_file, start_address, end_address, tag_specifier,
                                  policy, range_map)

        elif (byte == metadata_ops['DMD_TAG_POLICY_SYMBOL']):
            logging.critical("Saw policy symbol!\n")
            sys.exit(-1)
        elif (byte == metadata_ops['DMD_TAG_POLICY_RANGE']):
            logging.critical("Saw policy range!\n")
            sys.exit(-1)
            for _ in range(PTR_SIZE*3):
                #skip start, end, & 32-bit tag-type
                next(it)
        elif (byte == metadata_ops['DMD_TAG_POLICY_SYMBOL_RANKED']):
            logging.critical("Saw policy symbol ranked\n")
            sys.exit(-1)
        elif (byte == metadata_ops['DMD_TAG_POLICY_RANGE_RANKED']):
            sys.exit(-1)
            logging.critical("Saw policy symbol range ranked\n")
            #skip start, end ,tag category, rank, tag type
            for _ in range(PTR_SIZE*5):
                next(it)
        elif (byte == metadata_ops['DMD_END_BLOCK_WEAK_DECL_HACK']):
            logging.critical("saw end weak decl tag!\n")
            sys.exit(-1)
        elif (byte == metadata_ops['DMD_END_BLOCK']):
            end_address = bytes_to_uint(it, PTR_SIZE)
            logging.debug("saw end block tag range = " + hex(base_address) +
                          ":" + hex(base_address + end_address))
            if range_map:
                range_map.add_range(base_address, base_address + end_address, "COMPILER_GENERATED")
        elif (byte == metadata_ops['DMD_FUNCTION_RANGE']):
            start_address = bytes_to_uint(it, PTR_SIZE) + base_address
            end_address = bytes_to_uint(it, PTR_SIZE) + base_address + PTR_SIZE
            logging.debug("saw function range = " + hex(start_address) +
                          ":" + hex(end_address))
            if range_map:
                range_map.add_range(start_address, end_address, "COMPILER_GENERATED")
        else:
            logging.debug("Error: found unknown byte in metadata!" + hex(byte) + "\n")
            sys.exit(-1)


    return range_map
