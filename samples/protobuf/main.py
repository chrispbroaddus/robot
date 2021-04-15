import argparse
import logging
import tempfile

logging.basicConfig(level=logging.INFO)

import samples.protobuf.SampleMessage_pb2 as pb

def main():
    logging.info("protobuf sample application")

    sample = pb.Sample()
    sample.name = "sample name"

    with tempfile.TemporaryFile() as handle:
        print >> handle, sample.SerializeToString()

if __name__ == "__main__":
   main()
