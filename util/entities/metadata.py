#!/usr/bin/env python3
import os
import json
import urllib.request
import sys

OUTPUT_FOLDER = "entities"
CLIENT_VERSION = "1.20.1"

# https://stackoverflow.com/questions/13881092/download-progressbar-for-python-3


def reporthook(blocknum, blocksize, totalsize):
    readsofar = blocknum * blocksize
    if totalsize > 0:
        percent = readsofar * 1e2 / totalsize
        s = "\r%5.1f%% %*d / %d" % (
            percent, len(str(totalsize)), readsofar, totalsize)
        sys.stderr.write(s)
        if readsofar >= totalsize:  # near the end
            sys.stderr.write("\n")
    else:  # total size is unknown
        sys.stderr.write("read %d\n" % (readsofar))


if __name__ == "__main__":
    # Huge thanks to Bixilon for these entity mappings!
    if not os.path.isfile("entity_data_data_types.json"):
        print("Downloading entity_data_data_types.json")
        urllib.request.urlretrieve(
            "https://gitlab.bixilon.de/bixilon/pixlyzer-data/-/raw/master/version/{version}/entity_data_data_types.json".format(version=CLIENT_VERSION), "entity_data_data_types.json", reporthook)
    if not os.path.isfile("entities.json"):
        print("Downloading entities.json")
        urllib.request.urlretrieve(
            "https://gitlab.bixilon.de/bixilon/pixlyzer-data/-/raw/master/version/{version}/entities.json".format(version=CLIENT_VERSION), "entities.json", reporthook)
    with open("entity_data_data_types.json", 'rb') as entities_data_json:
        data = json.loads(entities_data_json.read())
        enum_entity_metadata = "typedef enum {\n"
        for i, key in enumerate(data.keys()):
            if i == len(data.keys()) - 1:
                enum_entity_metadata += "    ENTITY_DATA_" + \
                    data[key] + " = " + key + "\n"
            else:
                enum_entity_metadata += "    ENTITY_DATA_" + \
                    data[key] + " = " + key + ",\n"
        enum_entity_metadata += "} EntityDataMetadata;\n"
    with open("entities.json", 'rb') as entities_json:
        json_file = json.loads(entities_json.read())
        data = json_file["Entity"]["meta"]
        enum_player_metadata = "typedef enum {\n"
        for i, key in enumerate(data.keys()):
            if i == len(data.keys()) - 1:
                enum_player_metadata += "    " + \
                    str(key) + " = " + str(data[key]) + "\n"
            else:
                enum_player_metadata += "    " + \
                    str(key) + " = " + str(data[key]) + ",\n"
        enum_player_metadata += "} EntityMetadata;\n"
        data = json_file["minecraft:player"]["meta"]
        enum_player_metadata += "typedef enum {\n"
        for i, key in enumerate(data.keys()):
            if i == len(data.keys()) - 1:
                enum_player_metadata += "    " + \
                    str(key) + " = " + str(data[key]) + "\n"
            else:
                enum_player_metadata += "    " + \
                    str(key) + " = " + str(data[key]) + ",\n"
        enum_player_metadata += "} PlayerEntityMetadata;\n"
    if (not os.path.exists(OUTPUT_FOLDER)):
        os.mkdir(OUTPUT_FOLDER)
    with open(OUTPUT_FOLDER + "/entity_enums.h", "w") as x:
        x.write(enum_entity_metadata + enum_player_metadata)