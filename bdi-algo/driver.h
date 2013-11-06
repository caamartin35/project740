#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "compressor.h"
#include "types.h"

#define PATH_TO_TRACE_DIR "../traces/"
#define TOKEN_LOAD "LD"
#define TOKEN_STORE "ST"
#define CACHE_SIZE 1024 //(1 << 19)
#define CACHE_SETS 32   //512
#define CACHE_WAYS 4    //16

#endif