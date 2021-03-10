#ifndef __JEN_KINS_HASH_H__
#define __JEN_KINS_HASH_H__

#include <stdint.h>
#include <stddef.h>

#define hashsize(n) ((uint32_t)1<<(n))
#define hashmask(n) (hashsize(n)-1)
extern uint32_t hashlittle( const void *key, size_t length, uint32_t initval);
#endif
