import re
import sys

# config
STRUCT_ASSIGN = r'\b([A-Za-z0-9._]+)\b->\b([A-Za-z0-9._]+)\b = ([A-Za-z0-9._]+);'
DELIM_LOAD = ''

# start
print '>> Creating traceable file ...'

# get and parse arguments
args = sys.argv
if (len(args) < 2):
  print 'Error: You must supply an input file.'
  exit(1)

# found file
trace_file = args[1]
with open(trace_file, 'r') as f:
  for line in f:
    line = line.strip()
    print line
