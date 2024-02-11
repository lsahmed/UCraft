import json
import os
import re
import urllib.request
import sys

OUTPUT_FOLDER = "protocol"


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
    return merged_releases


# get all the packets from different versions and join them
def extract_packet_template(parsed, releases):
    S2Ctemplate = list()
    C2Stemplate = list()
    for version in releases:
        # newer versions of the game include play packets along with the config packets
        try:
            s2c_packets = parsed[str(version)]["packets"]["s2c"]["play"]
            c2s_packets = parsed[str(version)]["packets"]["c2s"]["play"]
        except:
            s2c_packets = parsed[str(version)]["packets"]["s2c"]
            c2s_packets = parsed[str(version)]["packets"]["c2s"]
        for s2c_packet in s2c_packets:
            if s2c_packet not in S2Ctemplate:
                S2Ctemplate.append(s2c_packet)
        for c2s_packet in c2s_packets:
            if c2s_packet not in C2Stemplate:
                C2Stemplate.append(c2s_packet)
    return [S2Ctemplate, C2Stemplate]


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
        S2Ctemplate, C2Stemplate = extract_packet_template(parsed, releases)
        c_s2c_header = "#ifndef _S2C_H\n#define _S2C_H\n\n"
        c_s2c_code = '#include "s2c.h"\n'

        c_c2s_code = '#include "c2s.h"\n#include "player.h"\n'
        c_c2s_header = "#ifndef _C2S_H\n#define _C2S_H\n"
        c_c2s_map = ""
        for idx, conversion in enumerate(S2Ctemplate):
            j = "#define S2C_PLAY_"
            j += str(conversion).upper()
            j += " %d\n" % (idx)
            c_s2c_header += j
        c_s2c_header += "\n"
        for version in releases:
            # newer versions of the game include play packets along with the config packets
            try:
                s2c_packets = parsed[str(version)]["packets"]["s2c"]["play"]
                c2s_packets = parsed[str(version)]["packets"]["c2s"]["play"]
            except:
                s2c_packets = parsed[str(version)]["packets"]["s2c"]
                c2s_packets = parsed[str(version)]["packets"]["c2s"]
            arranged_s2c = list()
            arranged_c2s = ["0"] * len(C2Stemplate)
            for templ in S2Ctemplate:
                if templ in s2c_packets:
                    arranged_s2c.append(int(s2c_packets.index(templ)))
                else:
                    arranged_s2c.append(0xFF)
            version_string = (
                re.sub("[^\\w\\s]", "_", releases[version][-1])
                + "_"
                + re.sub("[^\\w\\s]", "_", releases[version][0])
            )
            c_s2c_code += bin2header(arranged_s2c, "s2c_" + version_string)
            c_s2c_header += (
                "extern const unsigned char " + "s2c_" + version_string + "[];\n"
            )
            c_c2s_header += "extern const void *c2s_" + version_string + "[];\n"
            c_c2s_map += "const void *c2s_" + version_string + "[] = {\n"
            for c2spacket in c2s_packets:
                if c2spacket in C2Stemplate:
                    index = c2s_packets.index(c2spacket)
                    arranged_c2s[index] = "&PlayC2S_" + c2spacket
                else:
                    print("missing mapping", c2spacket)
            c_c2s_map += ",\n".join(arranged_c2s) + "\n};\n"
        for conversion in C2Stemplate:
            c_c2s_code += (
                "static void "
                + "PlayC2S_"
                + conversion
                + "(player_t *currentPlayer){}\n"
            )
        c_c2s_code += c_c2s_map
        # print(c_c2s_code)
        # c_s2c_header += "const unsigned int protocol_mapping_len = %d;" % (len(S2Ctemplate))
        c_s2c_header += "\n\n#define PLAYS2C_MAPPING_LEN %d" % (len(S2Ctemplate))
        c_s2c_header += "\n#endif"
        c_c2s_header += "\n\n#define PLAYC2S_MAPPING_LEN %d" % (len(C2Stemplate))
        c_c2s_header += "\n#endif"
        # print(c_c2s_header)
        # print(c_s2c_code)
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
