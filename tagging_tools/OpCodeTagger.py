import subprocess
from elftools.elf.elffile import ELFFile
from elftools.elf.constants import SH_FLAGS

def tag_op_codes(policy_dir, md_code, ef, taginfo_file_name):
     # tag the code for group tags
     for s in ef.iter_sections():
          if s['sh_flags'] & SH_FLAGS.SHF_EXECINSTR:
               section_addr_string = '0x{:08x}'.format(s['sh_addr'])
               proc = subprocess.Popen([md_code,
                                        policy_dir,
                                        #base_address_string,
                                        section_addr_string,
                                        taginfo_file_name],
                                       stdin=subprocess.PIPE,
               )
               proc.communicate(s.data())
               proc.wait()
