#ifndef __LIB_CONFIG_H__
#define __LIB_CONFIG_H__

// defines for testing
#define CONFIG_TESTx 1

// defines for L2 cache
#if CONFIG_TEST
  #define CONFIG_L2_SIZE             1024
  #define CONFIG_L2_WAYS             4
  #define CONFIG_L2_BLOCK_SIZE       32
#else
  #define CONFIG_L2_SIZE             (1 << 18)
  #define CONFIG_L2_WAYS             16
  #define CONFIG_L2_BLOCK_SIZE       32
#endif

#endif
