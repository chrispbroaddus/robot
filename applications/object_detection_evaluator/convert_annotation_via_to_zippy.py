import json
import argparse
from google.protobuf.json_format import MessageToJson
from packages.perception.proto import dataset_pb2
from packages.perception.proto import detection_pb2

def convert_annotation(input_via_json, output_zippy_json, dataset_name, dataset_version):

    zippyCategories = {"PERSON", "ANIMAL", "BICYCLE", "MOTORBIKE", "CAR", "BUS", "TRUCK"}
    dataset = dataset_pb2.Dataset()

    dataset.name = dataset_name
    dataset.version = dataset_version
    dataset.url = "EMPTY"
    dataset.annotation_types.append(dataset_pb2.AnnotationType.Value('BBOX_2D'))

    # Save client version
    jsonObj = MessageToJson(dataset)
    file = open(output_zippy_json +".client", "w")
    file.write(jsonObj)

    device = dataset.devices.add()

    with open(input_via_json) as data_file:
        viaFormatData = json.load(data_file)

    categoryCounter = dict()
    for category in zippyCategories:
        categoryCounter[category] = 0

    for viaFormatDataKey in viaFormatData:
        frame = device.frames.add()
        frame.path = "images/"
        frame.filename = viaFormatData[viaFormatDataKey]["filename"]

        for region in viaFormatData[viaFormatDataKey]["regions"]:

            viaFormatBbox = viaFormatData[viaFormatDataKey]["regions"][region]
            objClass = viaFormatBbox["region_attributes"]["class"]
            x = viaFormatBbox["shape_attributes"]["x"]
            y = viaFormatBbox["shape_attributes"]["y"]
            w = viaFormatBbox["shape_attributes"]["width"]
            h = viaFormatBbox["shape_attributes"]["height"]

            bbox = frame.boxes_2d.add()
            bbox.top_left_x = x
            bbox.top_left_y = y
            bbox.extents_x = w
            bbox.extents_y = h

            if objClass.upper() in zippyCategories:
                bbox.category.type = detection_pb2.Category.CategoryType.Value(objClass.upper())
                categoryCounter[objClass.upper()] += 1
            else:
                bbox.category.type = detection_pb2.Category.CategoryType.Value("UNKNOWN")
                categoryCounter["UNKNOWN"] += 1

    # Save server version
    jsonObj = MessageToJson(dataset)
    file = open(output_zippy_json, "w")
    file.write(jsonObj)

    print("=========== SUMMARY ===========")
    print(" - Input file       : %s" % input_via_json)
    print(" - Output file      : %s" % output_zippy_json)
    print(" - Categories")
    for key in categoryCounter:
        print("  -- %s : %d" % (key, categoryCounter[key]))

def parse_arguments():
    parser = argparse.ArgumentParser(description="annotation_converter")
    parser.add_argument('-i', '--input_via_json',
                        help='Input json in VIA(//thirdparty/via) json format.',
                        required=True)
    parser.add_argument('-o', '--output_zippy_json',
                        help='Output json in zippy(//packages/perception/proto/dataset.proto) format.',
                        default=True)
    parser.add_argument('-v', '--version',
                        help='Specified version.',
                        required=True)
    parser.add_argument('-n', '--name',
                        help='Name of the dataset.',
                        required=True)
    args = parser.parse_args()
    return (args.input_via_json, args.output_zippy_json, args.name, args.version)

def main():
    (input_via_json, output_zippy_json, dataset_name, dataset_version) = parse_arguments()
    convert_annotation(input_via_json, output_zippy_json, dataset_name, dataset_version)

if __name__ == "__main__":
    main()
