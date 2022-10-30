#ifndef HHBENCH_H
#define HHBENCH_H

#include <vector>
#include <fstream>
#include <time.h>
#include <vector>
#include <algorithm>

#include "Univmon.h"
#include "Elastic.h"
#include "CMHeap.h"
#include "CountHeap.h"
#include "SpaceSaving.h"

#include "OurHard.h"
#include "OurSoft.h"
#include "USS.h"

#include "SampleCoco.h"
#include "RuleCoco.h"
#include "Distributed.h"

#include "MMap.h"

bool comp(const std::pair<TUPLES,COUNT_TYPE>&a, const std::pair<TUPLES,COUNT_TYPE>&b) {
	return a.second > b.second;
}

class BenchMark{
public:

    BenchMark(std::string PATH, std::string name){
        fileName = name;

        result = Load(PATH.c_str());
        dataset = (BAO*)result.start;
        length = result.length / sizeof(BAO);

        std::cout << "stream length," << length << std::endl;

        for(uint64_t i = 0;i < length;++i){
            tuplesMp[*(TUPLES*)(dataset + i)] += 1;

            mp[0][dataset[i].srcIP_dstIP()] += 1;
            mp[1][dataset[i].srcIP_srcPort()] += 1;
            mp[2][dataset[i].dstIP_dstPort()] += 1;
            mp[3][dataset[i].srcIP()] += 1;
            mp[4][dataset[i].dstIP()] += 1;
        }

        // The following operations are not required for 
        // other experiments except experiment about D-CocoSketch with Mode Rule
        for(auto it : tuplesMp){
            TopK.push_back(it);
        }
        sort(TopK.begin(), TopK.end(), comp);
        int flow_num = 0;
        for(auto it = TopK.begin(); it != TopK.end(); ++it){
            topK_map[it->first] = flow_num;
            ++flow_num;
        }
    }

    ~BenchMark(){
        UnLoad(result);
    }

    void HHSingleBench(uint32_t MEMORY, double alpha){
#define HHOtherSketch CMHeap

        SingleAbstract<TUPLES>* tupleSketch;
        SingleAbstract<uint64_t>* sketch[5];


        for(uint32_t i = 1;i <= MAX_TRAFFIC;++i){
            uint32_t mem = MEMORY / i;

            tupleSketch = new HHOtherSketch<TUPLES>(mem);

            for(uint32_t j = 0;j < i - 1;++j){
                sketch[j] = new HHOtherSketch<uint64_t>(mem);
            }
            for(uint32_t j = 0;j < length;++j){
                switch(i){
                    case 6: sketch[4]->Insert(dataset[j].dstIP());
                    case 5: sketch[3]->Insert(dataset[j].srcIP());
                    case 4: sketch[2]->Insert(dataset[j].dstIP_dstPort());
                    case 3: sketch[1]->Insert(dataset[j].srcIP_srcPort());
                    case 2: sketch[0]->Insert(dataset[j].srcIP_dstIP());
                    default: tupleSketch->Insert(*(TUPLES*)(dataset + j));
                }
            }

            std::unordered_map<TUPLES, COUNT_TYPE> estTuple = tupleSketch->AllQuery();
            std::unordered_map<uint64_t, COUNT_TYPE> estMp[5];

            for(uint32_t j = 0;j < i - 1;++j){
                estMp[j] = sketch[j]->AllQuery();
            }

            COUNT_TYPE threshold = alpha * length;

            CompareHH(estTuple, tuplesMp, threshold, 1);

            for(uint32_t j = 0;j < i - 1;++j){
                CompareHH(estMp[j], mp[j], threshold, j + 2);
            }

            delete tupleSketch;
            for(uint32_t j = 0;j < i - 1;++j){
                delete sketch[j];
            }
        }
    }

    void HHMultiBench(uint32_t MEMORY, double alpha){
        MultiAbstract<TUPLES>* sketch = new OurHard<TUPLES>(MEMORY);

        for(uint32_t i = 0;i < length;++i){
            sketch->Insert(*(TUPLES*)(dataset + i));
        }

        std::unordered_map<TUPLES, COUNT_TYPE> estTuple = sketch->AllQuery();
        std::unordered_map<uint64_t, COUNT_TYPE> estMp[5];

        for(auto it = estTuple.begin();it != estTuple.end();++it){
            estMp[0][(it->first).srcIP_dstIP()] += it->second;
            estMp[1][(it->first).srcIP_srcPort()] += it->second;
            estMp[2][(it->first).dstIP_dstPort()] += it->second;
            estMp[3][(it->first).srcIP()] += it->second;
            estMp[4][(it->first).dstIP()] += it->second;
        }

        COUNT_TYPE threshold = alpha * length;

        CompareHH(estTuple, tuplesMp, threshold, 1);

        for(uint32_t i = 0;i < 5;++i){
            CompareHH(estMp[i], mp[i], threshold, i + 2);
        }

        delete sketch;
    }

    void HHSampleBench(Parameter* p, double alpha){
        
        SampleCoco<TUPLES>* sketch = new SampleCoco<TUPLES>(p);

        clock_t start, end;
        start = clock();
        for(uint32_t i = 0;i < length;++i){
            sketch->Insert(*(TUPLES*)(dataset + i));
        }
        end = clock();
        std::cout << "stream length," << length << std::endl;
        std::cout << "insert time(s)," << double(end - start) / CLOCKS_PER_SEC << std::endl;
        std::cout << "throughput(pps)," << length / (double(end - start) / CLOCKS_PER_SEC) << std::endl;
        std::cout << "throughput(Mpps)," << length / (double(end - start) / CLOCKS_PER_SEC) / 1e6<< std::endl;


        sketch->hash_flow();
        

        std::unordered_map<TUPLES, COUNT_TYPE> estTuple = sketch->AllQuery();
        std::unordered_map<uint64_t, COUNT_TYPE> estMp[5];

        for(auto it = estTuple.begin();it != estTuple.end();++it){
            estMp[0][(it->first).srcIP_dstIP()] += it->second;
            estMp[1][(it->first).srcIP_srcPort()] += it->second;
            estMp[2][(it->first).dstIP_dstPort()] += it->second;
            estMp[3][(it->first).srcIP()] += it->second;
            estMp[4][(it->first).dstIP()] += it->second;
        }

        COUNT_TYPE threshold = alpha * length;

        CompareHH(estTuple, tuplesMp, threshold, 1);

        for(uint32_t i = 0;i < 5;++i){
            CompareHH(estMp[i], mp[i], threshold, i + 2);
        }

        delete sketch;
    }

    void HHRuleBench(Parameter* p, double alpha){
        RuleCoco<TUPLES>* sketch = new RuleCoco<TUPLES>(p);

        clock_t start, end;
        start = clock();
        for(uint32_t i = 0;i < length;++i){
            sketch->ruleInsert(*(TUPLES*)(dataset + i), topK_map[*(TUPLES*)(dataset + i)]);
        }
        end = clock();
        std::cout << "stream length," << length << std::endl;
        std::cout << "insert time(s)," << double(end - start) / CLOCKS_PER_SEC << std::endl;
        std::cout << "throughput(pps)," << length / (double(end - start) / CLOCKS_PER_SEC) << std::endl;
        std::cout << "throughput(Mpps)," << length / (double(end - start) / CLOCKS_PER_SEC) / 1e6<< std::endl;


        sketch->hash_flow();
        

        std::unordered_map<TUPLES, COUNT_TYPE> estTuple = sketch->AllQuery();
        std::unordered_map<uint64_t, COUNT_TYPE> estMp[5];

        for(auto it = estTuple.begin();it != estTuple.end();++it){
            estMp[0][(it->first).srcIP_dstIP()] += it->second;
            estMp[1][(it->first).srcIP_srcPort()] += it->second;
            estMp[2][(it->first).dstIP_dstPort()] += it->second;
            estMp[3][(it->first).srcIP()] += it->second;
            estMp[4][(it->first).dstIP()] += it->second;
        }

        COUNT_TYPE threshold = alpha * length;

        CompareHH(estTuple, tuplesMp, threshold, 1);

        for(uint32_t i = 0;i < 5;++i){
            CompareHH(estMp[i], mp[i], threshold, i + 2);
        }

        delete sketch;
    }

    void HHDisBench(Parameter* p, double alpha){
        Distributed<TUPLES>* sketch = new Distributed<TUPLES>(p);

        for(uint32_t i = 0;i < length;++i){
            sketch->Insert(*(TUPLES*)(dataset + i));
        }

        std::unordered_map<TUPLES, COUNT_TYPE> estTuple = sketch->AllQuery();
        std::unordered_map<uint64_t, COUNT_TYPE> estMp[5];

        for(auto it = estTuple.begin();it != estTuple.end();++it){
            estMp[0][(it->first).srcIP_dstIP()] += it->second;
            estMp[1][(it->first).srcIP_srcPort()] += it->second;
            estMp[2][(it->first).dstIP_dstPort()] += it->second;
            estMp[3][(it->first).srcIP()] += it->second;
            estMp[4][(it->first).dstIP()] += it->second;
        }

        COUNT_TYPE threshold = alpha * length;

        CompareHH(estTuple, tuplesMp, threshold, 1);

        for(uint32_t i = 0;i < 5;++i){
            CompareHH(estMp[i], mp[i], threshold, i + 2);
        }

        delete sketch;
    }


private:
    std::string fileName;

    LoadResult result;

    BAO* dataset;
    uint64_t length;

    std::unordered_map<TUPLES, COUNT_TYPE> tuplesMp;
    std::unordered_map<uint64_t, COUNT_TYPE> mp[5];

    std::vector<std::pair<TUPLES, COUNT_TYPE> > TopK;
    std::unordered_map<TUPLES, int> topK_map;
    int flow_num;

    template<class T>
    void CompareHH(T mp, T record, COUNT_TYPE threshold, uint32_t key_type){
        double realHH = 0, estHH = 0, bothHH = 0, aae = 0, are = 0;

        for(auto it = record.begin();it != record.end();++it){
            bool real, est;
            double realF = it->second, estF = mp[it->first];

            real = (realF > threshold);
            est = (estF > threshold);

            realHH += real;
            estHH += est;

            if(real && est){
                bothHH += 1;
                aae += abs(realF - estF);
                are += abs(realF - estF) / realF;
            }
        }

        std::cout << std::endl << std::endl;

        std::cout << "key-type," << key_type << std::endl;
        std::cout << "threshold," << threshold << std::endl;

        std::cout << "Recall," << bothHH / realHH << std::endl;
        std::cout << "Precision," << bothHH / estHH << std::endl;

        std::cout << "aae," << aae / bothHH << std::endl;
        std::cout << "are," << are / bothHH << std::endl;
        std::cout << std::endl;
    }
};

#endif
