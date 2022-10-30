#include "BenchMark.h"
#include <string.h>
#include <unistd.h>
#include "MultiAbstract.h"
#include "function.h"


int main(int argc, char *argv[]){
    /* OurSoft, OurHard, USS, Univmon, Elastic, CMHeap, CountHeap, SpaceSaving */
    run(argc, argv);

    /* Dleft CocoSketch, Sample Mode */
    // runSample(argc, argv);

    /* Dleft CocoSketch, Rule Mode */
    // runRule(argc, argv);

    /* Distributed CocoSketch, Hard Version */
    // runDistributed(argc, argv);
    return 0;
}


std::string str(uint32_t n){
    std::string s = "";
    std::string tmp;
    do{
        tmp = ('0' + (n % 10));
        s = tmp + s;
        n /= 10;
    }while(n > 0);
    return s;
}

void printSamplePara(Parameter* para, double alpha){
    std::cout << "------------ D-Coco (Sample Mode) ------------\n";
    std::cout << "memory        = " << para->memory << std::endl;
    std::cout << "coco ratio    = " << para->coco_ratio << std::endl;
    std::cout << "hash num      = " << para->hash_num << std::endl;
    std::cout << "table num     = " << para->table_num << std::endl;
    std::cout << "sample        = " << (para->sample ? "yes" : "no") << std::endl;
    std::cout << "sample_length = " << para->sample_length << std::endl;
    std::cout << "sample_thres  = " << para->sample_threshold << std::endl;
    std::cout << "sample_ratio  = " << 1.0 * (para->sample_length - para->sample_threshold) / para->sample_length << std::endl;
    std::cout << "alpha         = " << alpha << std::endl;
    std::cout << "hash length   = " << para->hash_length << std::endl;
    std::cout << std::endl;
}

void printRulePara(Parameter* para, double alpha){
    std::cout << "------------ D-Coco (Rule Mode) ------------\n";
    std::cout << "memory        = " << para->memory << std::endl;
    std::cout << "alpha         = " << alpha << std::endl;
    std::cout << "coco ratio    = " << para->coco_ratio << std::endl;
    std::cout << "hash num      = " << para->hash_num << std::endl;
    std::cout << "table num     = " << para->table_num << std::endl;
    std::cout << "hash length   = " << para->hash_length << std::endl;
    std::cout << "K             = " << para->K << std::endl;
    std::cout << std::endl;
}

void printDistributedPara(Parameter* para, double alpha){
    std::cout << "------------ Distributed CocoSketch ------------\n";
    std::cout << "version     = " << para->version << std::endl;
    std::cout << "memory      = " << para->memory << std::endl;
    std::cout << "alpha       = " << alpha << std::endl;
    std::cout << "mode        = " << (para->mode == DIVIDE_K ? "divide into K" : "hash func 2") << std::endl;
    std::cout << "coco number = " << para->coco_num << std::endl;
    std::cout << "hash number = " << para->hash_num << std::endl << std::endl;
}

void run(int argc, char *argv[]){
    for(uint32_t i = 1;i < argc;++i) {
        std::cout << argv[i] << std::endl;
        BenchMark dataset(argv[i], "Dataset");
        dataset.HHMultiBench(500000, 0.0001);
        // dataset.HHSingleBench(500000, 0.0001);
    }
}


void runSample(int argc, char *argv[]){

    Parameter* para = new Parameter;

    // parameter setting
    para->memory = 1000000;
    para->hash_num = 1;
    para->table_num = 2; // sub table num 
    para->coco_ratio = 0.8; // the memory ratio of CocoSketch part
    para->sample = true;
    para->sample_length = 1000;
    para->sample_threshold = 994; // sample rate = (length - threshold) / length

    double alpha = 0.00010; 
    
    for(uint32_t i = 1; i < argc; ++i){
        std::cout << argv[i] << std::endl;
        BenchMark dataset(argv[i], "Dataset");

        int n = 1;
        while(para->memory <= 1000000){
            para->hash_length = para->memory * (1 - para->coco_ratio) / 21;
            printSamplePara(para, alpha);
            dataset.HHSampleBench(para, alpha);
            std::cout << "################################################\n\n\n";
            para->memory += 50000;
            ++n;
        }
    }

    delete para;
    return;
}

void runRule(int argc, char *argv[]){
    Parameter* para = new Parameter;

    para->memory = 1000000;
    para->hash_num = 1;
    para->table_num = 2;
    para->coco_ratio = 0.8;
    para->K = 8000;

    double alpha = 0.00010; 
    
    for(uint32_t i = 1; i < argc; ++i){
        std::cout << argv[i] << std::endl;
        BenchMark dataset(argv[i], "Dataset");
        int n = 1;
        while(para->K <= 8000){
            para->hash_length = para->memory * (1 - para->coco_ratio) / 21;
            printRulePara(para, alpha);
            dataset.HHRuleBench(para, alpha);
            std::cout << "################################################\n\n\n";
            para->K += 100;
            ++n;
        }
    }
    delete para;
}

void runDistributed(int argc, char *argv[]){
    Parameter* para = new Parameter;

    para->memory = 1000000;
    para->coco_num = 1;
    para->hash_num = 2;
    para->mode = DIVIDE_K; // DIVIDE_K : divide the total memory to K distributed CocoSketch
    para->version = "hard";

    double alpha = 0.00010; 
    
    for(uint32_t i = 1; i < argc; ++i){
        std::cout << argv[i] << std::endl;
        BenchMark dataset(argv[i], "Dataset");

        int n = 1;
        while(para->coco_num < 32){
            printDistributedPara(para, alpha);
            dataset.HHDisBench(para, alpha);
            std::cout << "################################################\n\n\n";
            para->coco_num = para->coco_num << 1;
            n++;
        }
    }
    delete para;
}

