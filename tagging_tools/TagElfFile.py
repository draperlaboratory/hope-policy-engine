#!/usr/bin/env python3

import os
import sys
import subprocess
import tempfile

def generate_tag_array(elfname, range_file, policy_meta_info, rv64):

    tag_array_file = tempfile.NamedTemporaryFile(mode='w+b', delete=False, prefix='tag_array_')
    tag_array_filename = tag_array_file.name
    length = policy_meta_info.get('MaxBit')

    if rv64:
        bytes_per_address = 8 # 64/8
        tool_prefix = "riscv64-unknown-elf-"
    else:
        bytes_per_address = 4 # 32/8
        tool_prefix = "riscv32-unknown-elf-"
    tag_array_bytes = [0]*bytes_per_address*(length+1)

    tag_array_file.write(int.to_bytes(length, byteorder='little', length=bytes_per_address))
    tag_array_file.write(bytearray(tag_array_bytes))
    tag_array_file.close()

    pout = subprocess.check_output([tool_prefix + 'objdump', '-h', elfname])

    if ".tag_array" in str(pout): # section exists, update the elf
        base_command = tool_prefix + "objcopy --update-section .tag_array=" + tag_array_filename + " " + elfname + " " + elfname
    else:
        base_command = tool_prefix + "objcopy --add-section .tag_array=" + tag_array_filename + " --set-section-flags .tag_array=readonly,data " + elfname + " " + elfname

    presult = subprocess.call(base_command.split(' '))

    if presult != 0:
        sys.exit(presult)

    os.remove(tag_array_filename)

    start_addr = ""
    pout = subprocess.check_output([tool_prefix + 'objdump', '-h', elfname])

    for line in str(pout).split('\\n'):
        if '.tag_array' in line:
            start_addr = (line.split()[3])

    start_addr = int(start_addr, 16)

    if start_addr:
        for m in policy_meta_info.get('Metadata'):
            mid = int(m.get('id'))
            range_file.write_range(start_addr + (mid*bytes_per_address) + bytes_per_address,
                                   start_addr + (mid*bytes_per_address) + (2*bytes_per_address),
                                   m.get('name'))
