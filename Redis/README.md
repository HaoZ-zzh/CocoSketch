# Redis Integration

## Environment

We implement and test the basic version and the hardware-friendly version on
* Ubuntu 20.04
* Redis version 5.0.7
* python 3.7.12


## Usage

* suppose the curernt working directory is `$REDIS_COCO`
* `make`
* start redis server by `redis-server`, and load module by start a redis client and run `module load $REDIS_COCO/CocoSketch.so`
* run the python scripts to measure the throughput
    * For basic version, run `python RedisControl.py -name SoftCoco -memory {memory} -d {depth}`
    * For hardware-friendly version, run `python RedisControl.py -name HardCoco -memory {memory} -d {depth}`