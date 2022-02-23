#!/usr/bin/env python3

import os
import sys
import subprocess
import tempfile

def generate_tag_array(elfname, range_file, policy_name, policy_meta_info, rv64):

    tag_array_file = tempfile.NamedTemporaryFile(mode='w+b', delete=False, prefix='tag_array_')
    tag_array_filename = tag_array_file.name
    length = policy_meta_info.get('MaxBit')

    if rv64:
        bytes_per_address = 8 # 64/8
        bfd_target = "elf64-littleriscv"
    else:
        bytes_per_address = 4 # 32/8
        bfd_target = "elf32-littleriscv"
    tool_prefix = "riscv64-unknown-elf-"
    tag_array_bytes = [0]*bytes_per_address*(length+1)

    tag_array_file.write(int.to_bytes(length, byteorder='little', length=bytes_per_address))
    tag_array_file.write(bytearray(tag_array_bytes))
    tag_array_file.close()

    pout = subprocess.check_output([tool_prefix + 'objdump', '-h', elfname])

    if ".tag_array" in str(pout): # section exists, update the elf
        base_command = tool_prefix + "objcopy --target=" + bfd_target + " --update-section .tag_array=" + tag_array_filename + " " + elfname + " " + elfname + "-" + policy_name
    else:
        base_command = tool_prefix + "objcopy --target=" + bfd_target + " --add-section .tag_array=" + tag_array_filename + " --set-section-flags .tag_array=readonly,data " + elfname + " " + elfname + "-" + policy_name

    presult = subprocess.call(base_command.split(' '))

    os.remove(tag_array_filename)

    if presult != 0:
        return presult

    start_addr = ""
    pout = subprocess.check_output([tool_prefix + 'objdump', '--target', bfd_target ,'-h', elfname+ "-" + policy_name])

    for line in str(pout).split('\\n'):
        if '.tag_array' in line:
            start_addr = (line.split()[3])

    start_addr = int(start_addr, 16)

    if start_addr:
        # metadata ids are 0-indexed, so we offset by 1 to allow .tag_array[0] to be the size.
        # iterate through addresses in .tag_array, tagging .tag_array[i+1] with the metadata with id i.
        for m in policy_meta_info.get('Metadata'):
            mid = int(m.get('id'))
            range_file.write_range(start_addr + (mid*bytes_per_address) + bytes_per_address,
                                   start_addr + (mid*bytes_per_address) + (2*bytes_per_address),
                                   m.get('name'))

    return presult
