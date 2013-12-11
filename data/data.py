class Result:
  def __init__(self, raw):
    raw = raw.strip()

    # populate
    lines = raw.split('\n')
    trace = lines[1].split('/')[-1].split('.trace')[0].strip()
    used = lines[3].split('=')[-1].split('-')
    hits = lines[5].split('=')[-1].split('-')

    self.trace = trace
    self.used = used[0].strip()
    self.util = used[1].strip()
    self.hits = hits[0].strip()
    self.hit_ratio = hits[1].strip()

  def __str__(self):
    util = '(' + self.util + ')'
    return '{:<20} {:>10} {:>10} {:>15}'.format(self.trace, self.used, util, self.hit_ratio)
