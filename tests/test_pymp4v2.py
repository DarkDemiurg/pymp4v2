import sys

import pymp4v2

if len(sys.argv) > 1:
    name = sys.argv[1]
else:
    print("No filename provided as argument.")
    sys.exit(1)

mp4_file = pymp4v2.MP4File(name)

print("Info:", mp4_file.get_info())
