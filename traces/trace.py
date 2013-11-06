import sys

# config
MAIN_DELIM = '_main:'

# start
print '>> Creating traceable file ...'

# get and parse arguments
args = sys.argv
if (len(args) < 2):
  print 'Error: You must supply an assembly file.'
  exit(1)

# found file
trace_file = args[1]
with open(trace_file, 'r') as f:
  for line in f:
    line = line.strip()
    print line
