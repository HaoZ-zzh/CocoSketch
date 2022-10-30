#include "Soft.h"
#include <regex.h>

#define REDIS_MODULE_TARGET

#ifdef REDIS_MODULE_TARGET
#include "util/redismodule.h"
#define CALLOC(count, size) RedisModule_Calloc(count, size)
#define FREE(ptr) RedisModule_Free(ptr)
#else
#define CALLOC(count, size) calloc(count, size)
#define FREE(ptr) free(ptr)
#endif

#define HASH(key, keylen, i) MurmurHash2(key, keylen, i)

uint32_t SoftCocoSketch::Query(void *o, const char *key, size_t key_len)
{
    return ((SoftCocoSketch *)o)->query(key, key_len);
}

void SoftCocoSketch::Create(uint32_t _d, uint32_t _w)
{
    assert(_w);
    assert(_d);

    total_packets = 0;
    w = _w;
    d = _d;
    
    counters = (uint32_t *)CALLOC(d * w, sizeof(uint32_t));
    keys = (char **)CALLOC(d * w, sizeof(char *));
    key_lens = (size_t *)CALLOC(d * w, sizeof(size_t));

    memset(counters, 0, sizeof(uint32_t) * d * w);
    memset(key_lens, 0, sizeof(size_t) * d * w);
}

void SoftCocoSketch::Destroy()
{
    FREE(keys);
    FREE(key_lens);
    FREE(counters);
}

SoftCocoSketch::SoftCocoSketch(uint32_t _d, uint32_t _w)
{
    Create(_d, _w);
}

SoftCocoSketch::~SoftCocoSketch()
{
    Destroy();
}

long long SoftCocoSketch::insert(const char *key, size_t key_len, uint32_t increment)
{
    assert(key);
    assert(key_len);

    total_packets += increment;

    uint32_t minimum = 0xffffffff;
    uint32_t minPos;

    for (uint32_t i = 0; i < d; ++i) {
        uint32_t pos = i * w + HASH(key, key_len, i) % w;
        if (key_len == key_lens[pos] && strncmp(key, keys[pos], key_len) == 0) {
            counters[pos] += increment;
            return 1;
        }
        if (counters[pos] < minimum) {
            minPos = pos;
            minimum = counters[pos];
        }
    }

    counters[minPos] += increment;
    if (rand() % counters[minPos] < increment) {
        if (key_lens[minPos] > 0)
            FREE(keys[minPos]);
        key_lens[minPos] = key_len;
        keys[minPos] = (char *) CALLOC(key_len, sizeof(char));
        memcpy(keys[minPos], key, key_len);
    }

    return 1;
}

uint32_t SoftCocoSketch::query(const char *key, size_t key_len)
{
    assert(key);
    assert(key_len);

    uint32_t result = 0;
    regex_t regex;
    int reti;
    if ((reti = regcomp(&regex, key, 0)) != 0)
        return -1;

    for (uint32_t i = 0; i < d * w; ++i) {
        if (key_lens[i] != 0 && regexec(&regex, keys[i], 0, NULL, 0) == 0)
            result += counters[i];
    }
    return result;
}

int SoftCocoSketch::query_total_packets() { return total_packets; }