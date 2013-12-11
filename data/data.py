import subprocess

#
# Class to hold result data.
#

class Result:

  # static members
  format = '{:<20} {:>10} {:>15} {:>15}'
  @staticmethod
  def header():
    return Result.format.format('TRACE', 'USED', 'UTIL', 'HIT%')

  @staticmethod
  def get(command):
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
    (output, err) = process.communicate()
    return Result(output)


  # instance members
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
    return Result.format.format(self.trace, self.used, self.util, self.hit_ratio)
