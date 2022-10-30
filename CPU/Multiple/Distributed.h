
#include "MultiAbstract.h"

template<typename DATA_TYPE>
class Distributed : public MultiAbstract<DATA_TYPE>{
public:

    typedef typename MultiAbstract<DATA_TYPE>::Counter Counter;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    Distributed(Parameter* p, std::string _name = "Distributed Coco"){
        this->name = _name;

        HASH_NUM = p->hash_num;
        COCO_NUM = p->coco_num;
        LENGTH = p->memory / COCO_NUM / HASH_NUM / sizeof(Counter);
        MODE = p->mode;

        counter = new Counter** [COCO_NUM];
        for(uint32_t i = 0; i < COCO_NUM; ++i){
            counter[i] = new Counter*[HASH_NUM];
            for(uint32_t j = 0; j < HASH_NUM; ++j){
                counter[i][j] = new Counter [LENGTH];
                memset(counter[i][j], 0, sizeof(Counter) * LENGTH);
            }
        }
    }

    ~Distributed(){
        for(uint32_t j = 0; j < COCO_NUM; ++j){
            for(uint32_t i = 0;i < HASH_NUM;++i){
                delete [] counter[j][i];
            }
            delete [] counter[j];
        }
        delete [] counter;
    }

    void Insert(const DATA_TYPE& item){
        uint32_t cocoPos = hash(item, 58) % COCO_NUM;

        if(MODE == HASH_FUNC_2){
            Insert_1(item);
            return;
        }

        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item, i) % LENGTH;
            counter[cocoPos][i][position].count += 1;
            if(randomGenerator() % counter[cocoPos][i][position].count == 0){
                counter[cocoPos][i][position].ID = item;
            }
        }
    }

    void Insert_1(const DATA_TYPE& item){
        uint32_t cocoPos_1 = hash(item, 58) % COCO_NUM;
        uint32_t cocoPos_2 = hash(item, 85) % COCO_NUM;

        uint32_t position = hash(item, 0) % LENGTH;
        counter[cocoPos_1][0][position].count += 1;
        if(randomGenerator() % counter[cocoPos_1][0][position].count == 0){
            counter[cocoPos_1][0][position].ID = item;
        }

        position = hash(item, 1) % LENGTH;
        counter[cocoPos_2][1][position].count += 1;
        if(randomGenerator() % counter[cocoPos_2][1][position].count == 0){
            counter[cocoPos_2][1][position].ID = item;
        }
    }

    HashMap AllQuery(){
        HashMap ret;
        if(MODE == HASH_FUNC_2){
            AllQuery_1(ret);
            return ret;
        }
        for(uint32_t c = 0; c < COCO_NUM; ++c){
            for(uint32_t i = 0;i < HASH_NUM;++i){
                for(uint32_t j = 0;j < LENGTH;++j){
                    if(ret.find(counter[c][i][j].ID) == ret.end()){
                        std::vector<COUNT_TYPE> estVec(HASH_NUM);
                        for(uint32_t k = 0;k < HASH_NUM;++k){
                            estVec[k] = counter[c][k][hash(counter[c][i][j].ID, k) % LENGTH].count;
                        }
                        ret[counter[c][i][j].ID] = Median(estVec, HASH_NUM);
                    }
                }
            }
        }

        return ret;
    }

    void AllQuery_1(HashMap& ret){
        for(uint32_t c = 0; c < COCO_NUM; ++c){
            for(uint32_t i = 0;i < HASH_NUM;++i){
                for(uint32_t j = 0;j < LENGTH;++j){
                    if(ret.find(counter[c][i][j].ID) == ret.end()){
                        uint32_t cnum = hash(counter[c][i][j].ID, 58) % COCO_NUM;
                        uint32_t pos = hash(counter[c][i][j].ID, 0) % LENGTH;
                        uint64_t v1 = counter[cnum][0][pos].count;

                        cnum = hash(counter[c][i][j].ID, 85) % COCO_NUM;
                        pos = hash(counter[c][i][j].ID, 1) % LENGTH;
                        uint64_t v2 = counter[cnum][1][pos].count;

                        ret[counter[c][i][j].ID] = (v1 + v2) / 2;
                    }
                }
            }
        }
    }

private:
    uint32_t LENGTH;
    uint32_t HASH_NUM;
    uint32_t COCO_NUM;
    uint32_t MODE;

    Counter*** counter;
};


