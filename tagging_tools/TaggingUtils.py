import os
import tempfile
import shutil

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

class RangeMap:
    def __init__(self):
        self.range_map = []

    def __contains__(self, key):
        start, end, tags = key
        return any(start >= s and end <= e for s, e, t in self.range_map)

    def __getitem__(self, index):
        return self.range_map[index]

    def sort(self):
        self.range_map = sorted(self.range_map)

    def add_range(self, start, end, tag=None):
        for curr_start, curr_end, tags in self.range_map:
            if curr_start == start and curr_end == end:
                tags.append(tag)
                return

        self.range_map.append((start, end, [tag]))

    def merge_ranges(self):
        rangemap = sorted(self.range_map)
        curr_s, curr_e, curr_tags = rangemap[0]
        for i, (s, e, tags) in enumerate(rangemap[1:], start=1):
            if s > curr_s:
                if e > curr_e:
                    rangemap[i-1] = (curr_s, s - 1, curr_tags)
                    rangemap[i] = (s, curr_e, curr_tags + tags)
                    rangemap.insert(i+1, (curr_e + 1, e, tags))
                else:
                    rangemap[i-i] = (curr_s, e, curr_tags + tags)
                    rangemap[i] = (e + 1, curr_e, curr_tags)
            elif s == curr_s:
                if e == curr_e:
                    rangemap[i-1] = (s, e, curr_tags + tags)
                    del rangemap[i]
                else:
                    rangemap[i-1] = (curr_s, curr_e, curr_tags + tags)
                    rangemap[i] = (curr_e + 1, e, tags)
            curr_s, curr_e, curr_tags = rangemap[i]

    def get_tags(self, addr):
        for curr_range, tags in self.range_map.items():
            (curr_start, curr_end) = curr_range
            if (addr >= curr_start and addr < curr_end):
                return tags
        return []

    def get_ranges(self, tag):
        return [curr_range for curr_range, tags in self.range_map.items()
                if tag in tags or (not tag and len(tags) == 0)]
