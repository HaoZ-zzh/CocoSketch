#ifndef OURABSTRACT_H
#define OURABSTRACT_H

#include <unordered_map>

#include <string.h>

#include "Util.h"

template<typename DATA_TYPE>
class MultiAbstract{
public:

    struct Counter{
        DATA_TYPE ID;
        COUNT_TYPE count;
    };
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    MultiAbstract(){}
    virtual ~MultiAbstract(){};

    std::string name;

    virtual void Insert(const DATA_TYPE& item) = 0;
    virtual HashMap AllQuery() = 0;
};

struct PARAMETER{
    uint32_t hash_num; // Coco hash num
    uint32_t memory; // total memory
    uint32_t table_num; // sub-table num
    uint32_t hash_length; // total number of flows that can be insert into sub-tables
    uint32_t K; // topK
    uint32_t sample_length; 
    uint32_t sample_threshold;
    uint32_t mode; // For Distributed CocoSketch
    uint32_t coco_num; // number of distributed CocoSketch
    double coco_ratio; // CocoSketch / (Coco size + Hash Table size = MEMORY)
    bool sample; // if take the sample mode or not
    std::string version; // hard or soft
};

typedef PARAMETER Parameter;

#define HASH_FUNC_2 1
#define DIVIDE_K 0

#endif
