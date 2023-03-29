#! /usr/bin/python
from argparse import ArgumentParser
from math import sin
from math import pi as PI

def gen_sine(args):
    h_scale = 2*PI*args.frequency/args.sample_rate
    for t in range(args.sample_rate // args.frequency):
        waveform = 0.5*args.bits_per_sample*(1 + sin(t * h_scale))
        print(int(waveform))
       
        
def main(args):
	gen_sine(args)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--sample-rate", type=int, default=44200)
    parser.add_argument("--frequency", type=int, default=440)   # A5
    parser.add_argument("--bits-per-sample", type=int, default=2**16)

    main(parser.parse_args())
