#include "Hard.h"
#include <regex.h>
#include <unordered_map>
#include <string>
#include <vector>

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

uint32_t HardCocoSketch::Query(void *o, const char *key, size_t key_len)
{
    return ((HardCocoSketch *)o)->query(key, key_len);
}

void HardCocoSketch::Create(uint32_t _d, uint32_t _w)
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

void HardCocoSketch::Destroy()
{
    FREE(keys);
    FREE(key_lens);
    FREE(counters);
}

HardCocoSketch::HardCocoSketch(uint32_t _d, uint32_t _w)
{
    Create(_d, _w);
}

HardCocoSketch::~HardCocoSketch()
{
    Destroy();
}

long long HardCocoSketch::insert(const char *key, size_t key_len, uint32_t increment)
{
    assert(key);
    assert(key_len);

    total_packets += increment;

    for (uint32_t i = 0; i < d; ++i) {
        uint32_t pos = i * w + HASH(key, key_len, i) % w;
        if (key_len == key_lens[pos] && strncmp(key, keys[pos], key_len) == 0) {
            counters[pos] += increment;
            continue;
        }
        
        counters[pos] += increment;
        if (rand() % counters[pos] < increment) {
            if (key_lens[pos] > 0) 
                FREE(keys[pos]);
            key_lens[pos] = key_len;
            keys[pos] = (char *) CALLOC(key_len, sizeof(char));
            memcpy(keys[pos], key, key_len);
        }
    }

    return 1;
}

uint32_t HardCocoSketch::query(const char *key, size_t key_len)
{
    assert(key);
    assert(key_len);

    uint32_t result = 0;
    regex_t regex;
    int reti;
    if ((reti = regcomp(&regex, key, 0)) != 0)
        return -1;

    if (d == 1) {
        for (uint32_t i = 0; i < w; ++i) {
            if (key_lens[i] != 0 && regexec(&regex, keys[i], 0, NULL, 0) == 0)
                result += counters[i];
        }    
    } else {
        std::unordered_map<std::string, uint32_t> est;
        for (uint32_t i = 0; i < d * w; ++i) {
            if (est.find(keys[i]) == est.end()) {
                std::vector<uint32_t> vec;
                for (uint32_t k = 0; k < d; ++k) {
                    uint32_t pos = k * w + HASH(key, key_len, k) % w;
                    if (key_lens[pos] == key_lens[i] && strncmp(keys[i], keys[pos], key_lens[i]) == 0)
                        vec.push_back(counters[pos]);
                }
                uint32_t median = 0;
                for (auto v : vec)
                    median += v;
                median /= vec.size();
                est[keys[i]] = median;
            }
        }

        for (auto p : est) {
            if (regexec(&regex, p.first.c_str(), 0, NULL, 0) == 0) {
                result += p.second;
            }
        }
    }
    
    return result;
}

int HardCocoSketch::query_total_packets() { return total_packets; }