#include <string.h>
#include <unistd.h>
#include "MultiAbstract.h"

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

void printPara(Parameter* para, double alpha){
    std::cout << "------------ HashCoco ------------\n";
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