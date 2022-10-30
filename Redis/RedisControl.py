from base64 import decode
from calendar import day_abbr
from pickle import FALSE
import string
from time import time
from redis import Redis
from math import log2
import csv
import argparse


def read_data_caida16():
    data = []
    with open("/share/datasets/CAIDA2016/CAIDA.dat", "rb") as file_in:
        while True:
            time = file_in.read(8)
            if not time:
                break
            _ = file_in.read(8)

            id = _.hex()
            id = int(id, 16)
            data.append(str(id))
            # data.append(str(_))
    # print(type(_))
    # print(data[:10])
    # data1 = ['7351735596318907619','7495401414670953371','8098596509523683178','4107829575238621661','3142382553768490378'] * 100000
    # print(data1[:10])
    return data[:500000]
    # return data1


def test_throughput(name, w, d):
    rd = Redis(decode_responses=True)
    rd.delete("test")
    print("d, w:" + str(d) + ' ' + str(w))
    pp = rd.execute_command(f"{name}.create", "test", d, w)

    data = read_data_caida16()
   

    time_start = time()
    res = rd.execute_command(f"{name}.insert", "test", *data)
    time_end = time()
    throughput_insert = len(data)/(time_end - time_start)

    print("Insert throughput is: ", throughput_insert)
    
    return throughput_insert


if __name__ == "__main__":
    print("start")
    res = []

    parser = argparse.ArgumentParser()
    parser.add_argument("-name", type=str, required=True, help="Software version or hardware version")
    parser.add_argument("-memory", type=int, required=True, help="the memory")
    parser.add_argument("-d", type=int, required=True, help="the d value")
    args = parser.parse_args()
    
    w = int(args.memory * 1000 / args.d / 12)
    test_throughput(args.name, w, args.d)
