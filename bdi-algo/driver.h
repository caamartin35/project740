#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "compressor.h"
#include "types.h"

#define PATH_TO_TRACE_DIR    "../traces/outputs/"
#define TOKEN_LOAD           "LD"
#define TOKEN_STORE          "ST"
#define CACHE_SIZE           1024 //(1 << 21)
#define CACHE_WAYS           4    //16
#define CACHE_BSIZE          32   //64

#endif