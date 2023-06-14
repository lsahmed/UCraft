#!/usr/bin/env python3
import os
import json
import urllib.request
import sys

OUTPUT_FOLDER = "blocks"
CLIENT_VERSION = "1.19.4"
HEADER_NAME = "blocks"
ENUM_NAME = "blocksDefaultState"
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
    # Huge thanks to Bixilon for these block mappings!
    if not os.path.isfile("blocks.json"):
        print("Downloading blocks.json")
        urllib.request.urlretrieve(
            f"https://gitlab.bixilon.de/bixilon/pixlyzer-data/-/raw/master/version/{CLIENT_VERSION}/blocks.json", "blocks.json", reporthook)
    with open("blocks.json", 'rb') as blocks_json:
        data = json.loads(blocks_json.read())
        header_code = f"#ifndef {HEADER_NAME.upper()}\n"
        header_code += f"#define {HEADER_NAME.upper()}\n\n"
        header_code += f"typedef enum {ENUM_NAME} \n{{\n"
        for key, value in data.items():
            default_state = value["default_state"]
            key = key.replace(":", "_")  # Replace colon with underscore
            header_code += f"    {key.upper()} = {default_state},\n"
        header_code += f"}} {ENUM_NAME};\n\n"
        header_code += f"#endif\n"
        if (not os.path.exists(OUTPUT_FOLDER)):
            os.mkdir(OUTPUT_FOLDER)
        with open(f"{OUTPUT_FOLDER}/{HEADER_NAME}.h", 'w') as header_file:
            header_file.write(header_code)
