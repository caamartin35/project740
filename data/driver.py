#!/usr/bin/python

import argparse
import glob

import data

# constants
DIR_TRACES = '../traces/outputs'
DIR_BASE = '../baseline'
DIR_BDI = '../bdi-algo'
EXT_BASE = 'trace'
EXT_POOL = 'pool.trace'
EXE_DRIVER = 'driver'
TRACE_TEST = 'test.'

# parse arguments
parser = argparse.ArgumentParser()
parser.add_argument('-summary', help='Only show the results summary.', action='store_true')
parser.add_argument('-tests', help='Also test and debug traces.', action='store_true')
parser.add_argument('-csv', nargs='?', const='results.csv')
args = parser.parse_args()

# starting the driver
print '>> Running simulator for all traces ...'

# get trace files
match = '/'.join([DIR_TRACES, '*.' + EXT_BASE])
files = glob.glob(match)
if not args.tests:
  files = [f for f in files if TRACE_TEST not in f]

# baseline jobs
print '.. Baseline traces ...'
print data.Result.summary_header()
base_data = [ ]
for filename in files:
  cargs = filename.split('/')[-1]
  command = '/'.join([DIR_BASE, EXE_DRIVER + ' ' + cargs])
  result = data.Result.get(command)
  base_data.append(result)
  print result

# base delta jobs
print '.. Base-Delta traces ... '
print data.Result.summary_header()
bdi_data = [ ]
for filename in files:
  cargs = filename.split('/')[-1]
  command = '/'.join([DIR_BDI, EXE_DRIVER + ' ' + cargs])
  result = data.Result.get(command)
  bdi_data.append(result)
  print result

if args.csv:
  print '.. Saving to CSV file ...',
  f = open(args.csv, 'w')
  # Baseline
  f.write('Baseline' + '\n')
  f.write(data.Result.csv_header() + '\n')
  for el in base_data:
    f.write(el.csv() + '\n')
  # Base-Delta
  f.write('Base-Delta' + '\n')
  f.write(data.Result.csv_header() + '\n')
  for el in bdi_data:
    f.write(el.csv() + '\n')
  f.close()
  print 'Done!'
