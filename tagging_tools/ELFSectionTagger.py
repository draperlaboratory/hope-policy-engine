from elftools.elf.elffile import ELFFile
from elftools.elf.constants import SH_FLAGS

# really not needed, but it's where you find RangeFile
import TaggingUtils

RWX_X = 'elf.Section.SHF_EXECINSTR'
RWX_R = 'elf.Section.SHF_ALLOC'
RWX_W = 'elf.Section.SHF_WRITE'

def generate_rwx_ranges(ef, range_file):
    sections = list(ef.iter_sections())
    for i,s in enumerate(sections):
          flags = s['sh_flags']
          start = s['sh_addr']
          end = start + s['sh_size']
          if (end % 4) != 0:
              end += 4 - (end % 4)
          if flags & SH_FLAGS.SHF_EXECINSTR:
               range_file.write_range(start, end, RWX_X)
               range_file.write_range(start, end, RWX_R)
               if (".init" in s.name or
                  ".exit" in s.name):
                   if i != len(sections)-1:
                       end = sections[i + 1]['sh_addr']
                   print('WX {0}: 0x{1:X} - 0x{2:X}'.format(s.name, start, end))
                   range_file.write_range(start, end, RWX_W)
               else:
                   print('X {0}: 0x{1:X} - 0x{2:X}'.format(s.name, start, end))
          elif flags & SH_FLAGS.SHF_WRITE:
               range_file.write_range(start, end, RWX_W)
               print('W {0}: 0x{1:X} - 0x{2:X}'.format(s.name, start, end))
          elif flags & SH_FLAGS.SHF_ALLOC:
               range_file.write_range(start, end, RWX_R)
               print('R {0}: 0x{1:X} - 0x{2:X}'.format(s.name, start, end))
