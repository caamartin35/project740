#include "driver.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::stringstream;

int main(int argc, char const *argv[]) {
  // check args
  if (argc < 2) {
    cout << "Usage: ./driver [trace_file]" << endl;
    return 1;
  }
  int extras = 1;
  int verbose = 0;

  // initialize
  cout << ">> Starting driver ... " << endl;

  // open the trace file
  string trace_file_name = std::string(PATH_TO_TRACE_DIR) + argv[1];
  cout << ">> Parsing file: " << trace_file_name << endl;
  ifstream trace_file(trace_file_name);

  // instantiate compressor
  Compressor compressor(CONFIG_L2_SIZE, CONFIG_L2_WAYS, CONFIG_L2_BLOCK_SIZE);

  // read in tokens
  string type;
  string address_str;
  size_t size;
  data_t data;
  while (trace_file >> type >> address_str >> size >> data) {
    // parse out the address
    pointer_t address;
    stringstream ss;
    ss << std::hex << address_str;
    ss >> address;

    // perform correct memory operation
    bool hit = false;
    if (type == TOKEN_LOAD) {
      hit = compressor.Load(address, size, data);
    } else if (type == TOKEN_STORE) {
      hit = compressor.Store(address, size, data);
    }

    // show the parsed line
    if (verbose > 0) {
      if (hit)
        cout << "[hit] ";
      else
        cout << "      ";
      cout << type << " 0x" << std::hex << address << " ";
      cout << std::dec << size << " " << data << endl;
    }

    // cycle the compressor
    compressor.Cycle();
  }

  // print out the results
  double hit_frac = 100.0 * compressor.hits / compressor.requests;
  double miss_frac = 100.0 * compressor.misses / compressor.requests;
  double util = 100.0 * compressor.used / compressor.size;
  cout << ">> Results: " << endl;
  cout << "   used = " << (compressor.used / 1024.0) << "KB - " << util << "%" << endl;
  cout << "   requests = " << compressor.requests << endl;
  cout << "   hits = " << compressor.hits << " - " << hit_frac << "%" <<  endl;
  cout << "   misses = " << compressor.misses << " - " << miss_frac << "%" << endl;
  cout << "   evictions = " << compressor.evictions << endl;

  // print out extras
  if (extras > 0) {
    BaseDeltaStats stats = compressor.Stats();
    cout << "   used_blocks = " << stats.used_blocks << endl;
    cout << "   zero_blocks = " << stats.zero_blocks << endl;
    cout << "   rep_blocks = " << stats.rep_blocks << endl;
    cout << "   medium_blocks = " << stats.medium_blocks << endl;
    cout << "   large_blocks = " << stats.large_blocks << endl;
  }

  // return
  return 0;
}
