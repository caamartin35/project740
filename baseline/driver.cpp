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

  // instantiate cache
  Cache cache(CONFIG_L2_SIZE, CONFIG_L2_WAYS, CONFIG_L2_BLOCK_SIZE);

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
      hit = cache.Load(address, size, data);
    } else if (type == TOKEN_STORE) {
      hit = cache.Store(address, size, data);
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

    // cycle the cache
    cache.Cycle();
  }

  // print out the results
  double hit_frac = 100.0 * cache.hits / cache.requests;
  double miss_frac = 100.0 * cache.misses / cache.requests;
  double util = 100.0 * cache.used / cache.size;
  cout << ">> Results: " << endl;
  cout << "   used = " << (cache.used / 1024.0) << "KB - " << util << "%" << endl;
  cout << "   requests = " << cache.requests << endl;
  cout << "   hits = " << cache.hits << " - " << hit_frac << "%" <<  endl;
  cout << "   misses = " << cache.misses << " - " << miss_frac << "%" << endl;
  cout << "   evictions = " << cache.evictions << endl;
  return 0;
}
