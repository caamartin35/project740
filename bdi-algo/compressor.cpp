#include "compressor.h"

using std::cout;
using std::endl;
using std::vector;

Compressor::Compressor(int size, int ways, int block_size) {
  // configure the cache dimensions
  this->size = size;
  this->ways = ways;
  this->block_size = block_size;
  this->sets = size / ways / block_size;

  // compute the address masks
  set_bits = log2(sets);
  bib_bits = log2(block_size);
  tag_bits = (sizeof(pointer_t) * BITS_IN_BYTE) - set_bits - bib_bits;

  // allocate cache memory
  setup_tags();
  data_store.resize(sets);
  for (int i = 0; i < data_store.size(); i++) {
    data_store[i].resize(block_size);
  }

  // initialize counters
  hits = 0;
  misses = 0;
  evictions = 0;
}

void Compressor::Load(pointer_t address, size_t size, data_t data) {
  // pointer_t index = get_set(address);
  // pointer_t tag = get_tag(address);
  // vector<Line>& set = cache[index];
  // Line* line = contains(set, tag);
}

void Compressor::Store(pointer_t address, size_t size, data_t data) {
  int index = get_set(address);
  vector<Tag>& tags = tag_store[index];
  pointer_t needle = get_tag(address);
  Tag *tag = contains(tags, needle);
  if (!tag) {
    misses++;
    insert(address, size, data);
  } else {
    hits++;
    tag->age = 0;
    tag->tag = needle;
    // uncompress, store, compress
  }
}

void Compressor::Cycle() {
  for (int i = 0; i < tag_store.size(); i++) {
    vector<Tag>* tags = &tag_store[i];
    for (int j = 0; j < tags->size(); j++) {
      tags->at(j).age++;
    }
  }
}


//
// BDI related helpers.
//
void Compressor::insert(pointer_t address, size_t size, data_t data) {
  // int index = get_set(address);
  int bib = get_bib(address);
  // set up the line for compression
  vector<byte_t> uncompressed;
  // vector<byte_t> compressed;
  write_bytes(data, bib, size, uncompressed);
}

void Compressor::compress(data_t data, std::vector<byte_t>* out_data, compression_t* out_compression) {

}

void Compressor::decompress(const Tag& tag, std::vector<byte_t>* out_data) {

}

void Compressor::write_bytes(data_t data, int offset, size_t length, std::vector<byte_t> line) {

}

Tag* Compressor::contains(vector<Tag>& tags, pointer_t needle) {
  for (int i = 0; i < tags.size(); i++) {
    Tag& tag = tags[i];
    if (tag.valid && tag.tag == needle)
      return &tag;
  }
  return NULL;
}


//
// Tag set up
//
void Compressor::setup_tags() {
  tag_store.resize(sets);
  for (int i = 0; i < tag_store.size(); i++) {
    tag_store[i].resize(2 * ways);  // twice as many tags
    for (int j = 0; j < tag_store[i].size(); j++) {
      Tag* tag = &tag_store[i][j];
      switch (j) {
        case 0: tag->SetInterval(6, 8); break;
        case 1: tag->SetInterval(5, 6); break;
        case 2: tag->SetInterval(2, 5); break;
        case 3: tag->SetInterval(0, 2); break;
        default: tag->SetInterval(SEGMENT_NONE, SEGMENT_NONE);
      }
    }
  }
}


//
// Cache related helpers.
//
pointer_t Compressor::get_tag(pointer_t address) {
  return ((address >> set_bits) >> bib_bits) & mask(tag_bits);
}

pointer_t Compressor::get_set(pointer_t address) {
  return (address >> bib_bits) & mask(set_bits);
}

pointer_t Compressor::get_bib(pointer_t address) {
  return address & mask(bib_bits);
}

pointer_t Compressor::mask(int len) {
  pointer_t mask = 0x0;
  while (len--) {
    mask = (mask << 1) | 0x1;
  }
  return mask;
}

int Compressor::log2(int n) {
  int i = 0;
  while (n >>= 1) {
    i++;
  }
  return i;
}


//
// Debugging functions
//
void Compressor::Print() const {
  for (int i = 0; i < sets; i++) {
    const vector<Tag>& tags = tag_store[i];
    cout << "set[" << i << "] tags: ";
    for (int j = 0; j < tags.size(); j++) {
      const Tag& tag = tags[j];
      tag.Print();
      cout << " ";
    }
    cout << endl;
  }
}
