#! /usr/bin/python
from argparse import ArgumentParser
import struct
from sys import stdin


def main(args):
    with open(args.output_file, 'wb') as f:
        for line in stdin:
            f.write(struct.pack("<I", int(line)))

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--output-file", type=str, default="binary_file.bin")

    main(parser.parse_args())
