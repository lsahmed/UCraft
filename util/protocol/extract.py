import json
import os
import re
import urllib.request
import sys

OUTPUT_FOLDER = "protocol"

PACKET_DIRECTIONS = ["c2s", "s2c"]
PACKET_STATES = ["play", "configuration"]


# https://stackoverflow.com/questions/13881092/download-progressbar-for-python-3
def reporthook(blocknum, blocksize, totalsize):
    readsofar = blocknum * blocksize
    if totalsize > 0:
        percent = readsofar * 1e2 / totalsize
        s = "\r%5.1f%% %*d / %d" % (percent, len(str(totalsize)), readsofar, totalsize)
        sys.stderr.write(s)
        if readsofar >= totalsize:  # near the end
            sys.stderr.write("\n")
    else:  # total size is unknown
        sys.stderr.write("read %d\n" % (readsofar))


# https://nachtimwald.com/2019/10/09/python-binary-to-c-header/
def bin2header(data, var_name="var"):
    out = []
    out.append("const unsigned char {var_name}[] = {{".format(var_name=var_name))
    l = [data[i : i + 12] for i in range(0, len(data), 12)]
    for i, x in enumerate(l):
        line = ", ".join(["0x{val:02x}".format(val=c) for c in x])
        out.append(
            "  {line}{end_comma}".format(
                line=line, end_comma="," if i < len(l) - 1 else ""
            )
        )
    out.append("};\n")
    # out.append('const unsigned int {var_name}_len = {data_len};\n'.format(var_name=var_name, data_len=len(data)))
    return "\n".join(out)


# look for releases and merge if they have the same protocol version
def merge_releases(parsed):
    merged_releases = {}
    for x in parsed:
        try:
            if parsed[x]["type"] == "release":
                name = parsed[x]["name"]
                # if(not name == "1.18.2"):
                #    continue
                version = 0
                if isinstance(parsed[x]["packets"], int):
                    version = parsed[x]["packets"]
                else:
                    version = int(x)
                if version in merged_releases:
                    merged_releases[version].append(name)
                else:
                    merged_releases[version] = [name]
        except:
            continue
    for version in merged_releases:
        if type(parsed[str(version)]["packets"]["c2s"]) == dict:
            playC2S = parsed[str(version)]["packets"]["c2s"]["play"]
            playS2C = parsed[str(version)]["packets"]["s2c"]["play"]
            configC2S = parsed[str(version)]["packets"]["c2s"]["configuration"]
            configS2C = parsed[str(version)]["packets"]["s2c"]["configuration"]
        else:
            playC2S = parsed[str(version)]["packets"]["c2s"]
            playS2C = parsed[str(version)]["packets"]["s2c"]
            configC2S = []
            configS2C = []
        merged_releases[version] = {
            "versions": merged_releases[version],
            "packets": {
                "c2s": {"play": playC2S, "configuration": configC2S},
                "s2c": {"play": playS2C, "configuration": configS2C},
            },
        }
    return merged_releases


# get all the packets from different versions and join them
def extract_packet_template(releases):
    packet_template = {
        "c2s": {"play": [], "configuration": []},
        "s2c": {"play": [], "configuration": []},
    }
    for version in releases:
        for direction in PACKET_DIRECTIONS:
            for state in PACKET_STATES:
                for packet in releases[version]["packets"][direction][state]:
                    if packet not in packet_template[direction][state]:
                        packet_template[direction][state].append(packet)
    return packet_template


if __name__ == "__main__":
    # Huge thanks to Bixilon for these protocol mappings!
    if not os.path.isfile("versions.json"):
        print("Downloading versions.json")
        urllib.request.urlretrieve(
            "https://gitlab.bixilon.de/bixilon/minosoft/-/raw/master/src/main/resources/assets/minosoft/mapping/versions.json",
            "versions.json",
            reporthook,
        )
    with open("versions.json", "rb") as versions_json:
        parsed = json.loads(versions_json.read())
        releases = merge_releases(parsed)
        packetTemplate = extract_packet_template(releases)
        c_s2c_header = "#ifndef _S2C_H\n#define _S2C_H\n\n"
        c_s2c_code = '#include "s2c.h"\n'

        c_c2s_code = '#include "c2s.h"\n#include "player.h"\n'
        c_c2s_header = "#ifndef _C2S_H\n#define _C2S_H\n"
        c_c2s_map = ""
        for state in PACKET_STATES:
            for idx, conversion in enumerate(packetTemplate["s2c"][state]):
                j = "#define S2C_" + state.upper() + "_"
                j += str(conversion).upper()
                j += " %d\n" % (idx)
                c_s2c_header += j
            c_s2c_header += (
                "\n#define S2C_"
                + state.upper()
                + "_MAPPING_LEN %d\n\n" % (len(packetTemplate["s2c"][state]))
            )
            for conversion in packetTemplate["c2s"][state]:
                c_c2s_code += (
                    "static void "
                    + state.capitalize()
                    + "C2S_"
                    + conversion
                    + "(player_t *currentPlayer){}\n"
                )
            c_c2s_code += "\n"
        for state in PACKET_STATES:
            for version in releases:
                # check if the packet list is empty
                if not releases[version]["packets"]["s2c"][state]:
                    continue
                # the version contains the oldest client supported on the protocol to the newest
                version_string = (
                    re.sub("[^\\w\\s]", "_", releases[version]["versions"][-1])
                    + "_"
                    + re.sub("[^\\w\\s]", "_", releases[version]["versions"][0])
                )
                arranged_s2c = list()
                # look up the packet in the template and append how far is the index from the releases
                for templ in packetTemplate["s2c"][state]:
                    if templ in releases[version]["packets"]["s2c"][state]:
                        arranged_s2c.append(
                            int(releases[version]["packets"]["s2c"][state].index(templ))
                        )
                    else:
                        arranged_s2c.append(0xFF)
                c_s2c_code += bin2header(
                    arranged_s2c, "s2c_" + state + "_" + version_string
                )
                c_s2c_header += (
                    "extern const unsigned char "
                    + "s2c_"
                    + state
                    + "_"
                    + version_string
                    + "[];\n"
                )
                # c2s starts here
                arranged_c2s = ["0"] * len(packetTemplate["c2s"][state])
                c_c2s_header += (
                    "extern const void *c2s_" + state + "_" + version_string + "[];\n"
                )
                c_c2s_map += (
                    "const void *c2s_" + state + "_" + version_string + "[] = {\n"
                )
                # here the order of the C2S function matters, so the packet index is compared by the template and placed accordingly
                for c2spacket in releases[version]["packets"]["c2s"][state]:
                    if c2spacket in packetTemplate["c2s"][state]:
                        index = releases[version]["packets"]["c2s"][state].index(
                            c2spacket
                        )
                        arranged_c2s[index] = (
                            "&" + state.capitalize() + "C2S_" + c2spacket
                        )
                    else:
                        print("missing mapping", c2spacket)
                c_c2s_map += ",\n".join(arranged_c2s) + "\n};\n"
            c_c2s_code += c_c2s_map
            # print(c_c2s_code)
            # c_s2c_header += "const unsigned int protocol_mapping_len = %d;" % (len(S2CPlayTemplate))
            c_c2s_header += (
                "\n#define C2S_"
                + state.upper()
                + "_MAPPING_LEN %d\n\n" % (len(packetTemplate["c2s"][state]))
            )
        c_s2c_header += "\n#endif"
        c_c2s_header += "\n#endif"
        if not os.path.exists(OUTPUT_FOLDER):
            os.mkdir(OUTPUT_FOLDER)
        with open(OUTPUT_FOLDER + "/s2c.h", "w") as x:
            x.write(c_s2c_header)
        with open(OUTPUT_FOLDER + "/s2c.c", "w") as x:
            x.write(c_s2c_code)
        with open(OUTPUT_FOLDER + "/c2s.h", "w") as x:
            x.write(c_c2s_header)
        with open(OUTPUT_FOLDER + "/c2s.c", "w") as x:
            x.write(c_c2s_code)
