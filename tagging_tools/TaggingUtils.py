import os
import tempfile

class RangeFile:
     def __init__(self):
          self.file = tempfile.NamedTemporaryFile(mode='a', delete = False, prefix='ranges_');
     def write_range(self, start, end, tag):
          self.file.write('0x%x 0x%x %s\n' % (start, end, tag))
     def finish(self):
          self.file.close();
     def name(self):
          if self.file is None:
               raise Exception('file does not exist')
          return self.file.name
     def done(self):
          if self.file is not None:
               try:
                    os.remove(self.file.name)
                    self.file = None
               except:
                    pass
     def print(self):
          print(self.file.name)
          with open(self.file.name, 'r') as f:
               for l in f.readlines():
                    print(l)
     def __del__(self):
          self.done()

