#!/env/python3

# RGFX Skybox wrapper script

import sys, subprocess, os
from pathlib import Path

command = '"' + sys.path[0] + "/n64graphics" + '"' + " -s u8 -i " + sys.argv[2] + "_ -g " + sys.argv[1] + " -f rgba16"
os.system(command)

outName = Path(sys.argv[1]).stem

fileOut = open(sys.argv[2], "w")
fileOut.write("#include \"types.h\"\n")
fileOut.write("#include \"make_const_nonconst.h\"\n")
fileOut.write("ALIGNED8 const Texture " + outName + "_skybox_texture[] = {\n")
fileOut.write("#include \"" + outName + "_skybox.c_\"\n")
fileOut.write("};\n")
fileOut.close()
