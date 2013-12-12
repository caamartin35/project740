import os
import subprocess

#
# Class to hold result data.
#

class Result:

  # static members
  summary_format = '{:<20} {:<10} {:>15} {:>15} {:>15}'
  csv_format = '{:}, {:}, {:}, {:}, {:}, {:}'
  @staticmethod
  def summary_header():
    return Result.summary_format.format('TRACE', 'LENGTH', 'USED', 'UTIL', 'HIT%')

  @staticmethod
  def csv_header():
    return Result.csv_format.format('TRACE', 'LENGTH (KB)', 'USED (KB)', 'REQUESTS', 'HITS', 'MISSES')


  @staticmethod
  def get(command):
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
    (output, err) = process.communicate()
    return Result(output)


  # instance members
  def __init__(self, raw):
    raw = raw.strip()

    # parse
    lines = raw.split('\n')
    path = lines[1].split(':')[-1].strip()
    trace = path.split('/')[-1].split('.trace')[0].strip()
    used = lines[3].split('=')[-1].split('-')
    requests = lines[4].split('=')[-1].strip()
    hits = lines[5].split('=')[-1].split('-')
    misses = lines[6].split('=')[-1].split('-')

    # store
    self.trace = trace
    self.size = str(os.path.getsize(path) / 1024) + 'KB'
    self.used = used[0].strip()
    self.util = used[1].strip()
    self.requests = requests
    self.hits = hits[0].strip()
    self.hit_ratio = hits[1].strip()
    self.misses = misses[0].strip()
    self.miss_ratio = misses[1].strip()

  def __str__(self):
    return self.summary()

  def summary(self):
    return Result.summary_format.format(self.trace, self.size, self.used, self.util, self.hit_ratio)

  def csv(self):
    return Result.csv_format.format(self.trace, self.size.strip('KB'), self.used.strip('KB'), self.requests, self.hits, self.misses)

