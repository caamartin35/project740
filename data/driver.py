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

# parse arguments
parser = argparse.ArgumentParser()
parser.add_argument('-all', help='Run all traces.', action='store_true')
parser.add_argument('-pool', help='Only run traces using memory pooling.', action='store_true')
args = parser.parse_args()

# starting the driver
print '>> Running simulator for all traces ...'

# get trace files
match = '/'.join([DIR_TRACES, '*.' + EXT_BASE])
files = glob.glob(match)

# baseline jobs
print '.. Baseline traces ...'
print data.Result.header()
base_data = [ ]
for filename in files:
  args = filename.split('/')[-1]
  command = '/'.join([DIR_BASE, EXE_DRIVER + ' ' + args])
  result = data.Result.get(command)
  base_data.append(result)
  print result

# base delta jobs
print '.. Base-Delta traces ...'
print data.Result.header()
bdi_data = [ ]
for filename in files:
  args = filename.split('/')[-1]
  command = '/'.join([DIR_BDI, EXE_DRIVER + ' ' + args])
  result = data.Result.get(command)
  bdi_data.append(result)
  print result
