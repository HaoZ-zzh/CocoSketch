#ifndef _HARD_H_
#define _HARD_H_

#include "util/MurmurHash2.h"

#include <assert.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <string.h>

class HardCocoSketch {
public:
    int total_packets;
    uint32_t d, w;
    uint32_t *counters;
    char **keys;
    size_t *key_lens;

    void Create(uint32_t _d, uint32_t _w);
    void Destroy();
    HardCocoSketch(uint32_t _d, uint32_t _w);
    ~HardCocoSketch();
    long long insert(const char *key, size_t key_len, uint32_t increment);
    uint32_t query(const char *key, size_t key_len);
    static uint32_t Query(void *o, const char *key, size_t key_len);
    int query_total_packets();
};

#endif