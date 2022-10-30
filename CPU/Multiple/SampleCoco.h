
#include "MultiAbstract.h"

template<typename DATA_TYPE>
class SampleCoco : public MultiAbstract<DATA_TYPE>{
public:

    typedef typename MultiAbstract<DATA_TYPE>::Counter Counter;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    SampleCoco(Parameter* p, std::string _name = "SampleCoco"){
        this->name = _name;

        SAMPLE = p->sample;
        SAMPLE_LENGTH = p->sample_length;
        SAMPLE_THRESHOLD = p->sample_threshold;
        HASH_NUM = p->hash_num;
        TABLE_NUM = p->table_num;
        COCO_LENGTH = p->memory * p->coco_ratio / HASH_NUM / sizeof(Counter);
        TABLE_LENGTH = p->memory * (1 - p->coco_ratio) / TABLE_NUM / sizeof(Counter);

        std::cout << "COCO_LENGTH   = " << COCO_LENGTH << std::endl;
        std::cout << "TABEL_LENGTH  = " << TABLE_LENGTH << std::endl;
        std::cout << "hash length," << p->hash_length << std::endl;
        
        counter = new Counter* [HASH_NUM];
        table = new Counter*[TABLE_NUM];

        for(uint32_t i = 0;i < HASH_NUM;++i){
            counter[i] = new Counter [COCO_LENGTH];
            memset(counter[i], 0, sizeof(Counter) * COCO_LENGTH);
        }
        for(uint32_t i = 0; i < TABLE_NUM; ++i){
            table[i] = new Counter [TABLE_LENGTH];
            memset(table[i], 0, sizeof(Counter) * TABLE_LENGTH);
        }
    }

    ~SampleCoco(){
        for(uint32_t i = 0;i < HASH_NUM;++i){
            delete [] counter[i];
        }
        for(uint32_t i = 0; i < TABLE_NUM; ++i){
            delete [] table[i];
        }
        delete [] counter;
        delete [] table;
    }

    void Insert(const DATA_TYPE& item){
        uint32_t samplePos = hash(item, 99) % SAMPLE_LENGTH;
        if(SAMPLE && samplePos < SAMPLE_THRESHOLD){
            CocoInsert(item);
            return;
        }
        for(uint32_t i = 0; i < TABLE_NUM; ++i){
            uint32_t pos = hash(item, i) % TABLE_LENGTH;
            if(table[i][pos].count == 0){
                table[i][pos].ID = item;
                table[i][pos].count = 1;
                hash_sample[item] = 2;
                return;
            }
            if(table[i][pos].ID == item){
                table[i][pos].count++;
                return;
            }
        }
        hash_sample[item] = 1;
        CocoInsert(item);
    }

    void CocoInsert(const DATA_TYPE& item){
        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item, i+TABLE_NUM) % COCO_LENGTH;
            counter[i][position].count += 1;
            if(randomGenerator() % counter[i][position].count == 0){
                counter[i][position].ID = item;
            }
        }
    }

    HashMap AllQuery(){
        HashMap ret;

        for(uint32_t i = 0;i < HASH_NUM;++i){
            for(uint32_t j = 0;j < COCO_LENGTH;++j){
                ret[counter[i][j].ID] = counter[i][j].count;
            }
        }

        for(uint32_t i = 0; i < TABLE_NUM; ++i){
            for(uint32_t j = 0; j < TABLE_LENGTH; ++j){
                ret[table[i][j].ID] += table[i][j].count;
            }
        }

        return ret;
    }

    void hash_flow(){
        int num1 = 0;
        int num2 = 0;
        for(auto it = hash_sample.begin(); it != hash_sample.end(); ++it){
            if(it->second == 2){
                ++num1;
                ++num2;
            }
            else if(it->second == 1){
                ++num2;
            }
        }
        std::cout << "hash attempt num," << num2 << std::endl;
        std::cout << "hash flow num," << num1 << std::endl;
    }

private:
    uint32_t COCO_LENGTH;
    uint32_t TABLE_LENGTH;
    uint32_t TABLE_NUM;
    uint32_t HASH_NUM;
    bool SAMPLE;
    uint32_t SAMPLE_LENGTH;
    uint32_t SAMPLE_THRESHOLD;

    Counter** counter;
    Counter** table;

    HashMap hash_sample;
};


