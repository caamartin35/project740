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
  int verbose = 0;

  // initialize
  cout << ">> Starting driver ... " << endl;

  // open the trace file
  string trace_file_name = std::string(PATH_TO_TRACE_DIR) + argv[1];
  cout << ">> Parsing file: " << trace_file_name << endl;
  ifstream trace_file(trace_file_name);

  // instantiate compressor
  int cache_size = (1 << 20); // 512;
  int cache_ways = 2; // 16;
  int cache_block_size = 32;
  Compressor compressor(cache_size, cache_ways, cache_block_size);

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
  return 0;
}
