## Base-Delta Compression with Memory Pooling

This is a research project for CMU 18-740.


### Config

You can alter various cache paramters in the `lib/config.h` file. If you modify this file, you will have to `make clean` and rebuild each simulator.


### Data

In the `data/` directory you can use `driver.py` to run all traces on each simulator. The output is very readable. To save the results, just redirect `stdout` to a new text file.

The driver has a few built in options.

- `-h` - Print usage and list options.
- `-csv` - Output the results in CSV format to the given file (`results.csv` by default).
- `-test` - Also run traces prefixed with `test.`


### Traces

Currently the repository tracks trace files. This is so that you don't have to rebuild each test and run them every time you `git clone`.

If you will to add or modify a trace, follow the folder conventions under the `traces/` directory, and it should be pretty simple. The easiest this to do is to `cp -r` an existing test and work from there.
