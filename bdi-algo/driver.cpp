#include "driver.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::stringstream;

int main(int argc, char const *argv[]) {
  // initialize
  cout << ">> Starting driver ... " << endl;
  Compressor compressor(CACHE_SIZE, CACHE_SETS, CACHE_WAYS);

  // open the trace file
  string trace_file_name = std::string(PATH_TO_TRACE_DIR) + "test.trace";
  cout << ">> Parsing file: " << trace_file_name << endl;
  ifstream trace_file(trace_file_name);

  // read in tokens
  stringstream ss;
  string type;
  string address_str;
  size_t size;
  data_t data;
  while (trace_file >> type >> address_str >> size) {
    // parse out the address
    pointer_t address;
    ss << std::hex << address_str;
    ss >> address;
    // show the parsed line
    cout << type << " 0x" << std::hex << address << " " << std::dec << size;
    if (type == TOKEN_STORE) {
      trace_file >> data;
      cout << " " << data;
    }
    cout << endl;
    // perform correct memory operation
  }

  return 0;
}
