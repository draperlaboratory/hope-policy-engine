from elftools.elf.elffile import ELFFile
from elftools.elf.constants import SH_FLAGS

# really not needed, but it's where you find RangeFile
import TaggingUtils

RWX_X = 'elf.Section.SHF_EXECINSTR'
RWX_R = 'elf.Section.SHF_ALLOC'
RWX_W = 'elf.Section.SHF_WRITE'

def generate_rwx_ranges(ef, range_file):
     for s in ef.iter_sections():
          flags = s['sh_flags']
          start = s['sh_addr']
          end = start + s['sh_size']
          if flags & SH_FLAGS.SHF_EXECINSTR:
               range_file.write_range(start, end, RWX_X)
               range_file.write_range(start, end, RWX_R)
               print('X {0}: 0x{1:X}'.format(s.name, start))
          elif flags & SH_FLAGS.SHF_WRITE:
               range_file.write_range(start, end, RWX_W)
               print('W {0}: 0x{1:X}'.format(s.name, start))
          elif flags & SH_FLAGS.SHF_ALLOC:
               range_file.write_range(start, end, RWX_R)
               print('R {0}: 0x{1:X}'.format(s.name, start))
