import os
import sys

sys.path.insert(0, './build')

import pymp4v2

pymp4v2.raw.MP4LogSetLevel(pymp4v2.MP4_LOG_WARNING)

FILE_1 = '/home/dima/1/89.mp4'
FILE_2 = '/home/dima/1/104406.mp4'

print('=== Begin tests')

print('===== Test MP4Read')
h = pymp4v2.raw.MP4Read(FILE_1)
h2 = pymp4v2.raw.MP4Read(FILE_2)
print(f'MP4FileHandle = {h} {h2}')
print(pymp4v2.raw.MP4GetFilename(h), pymp4v2.raw.MP4GetFilename(h2))
pymp4v2.raw.MP4Close(h)
pymp4v2.raw.MP4Close(h2)

print('===== Test MP4Modify')
h = pymp4v2.raw.MP4Modify(FILE_1)
print(f'MP4FileHandle = {h}')
print(pymp4v2.raw.MP4GetFilename(h))
pymp4v2.raw.MP4Close(h)

print('===== Test MP4Dump')
h = pymp4v2.raw.MP4Read(FILE_1)
print(f'MP4FileHandle = {h}')
print(pymp4v2.raw.MP4Dump(h))
print('========== ')
print(pymp4v2.raw.MP4Dump(h, True))
pymp4v2.raw.MP4Close(h)

print('===== Test context manager')
with pymp4v2.raw.MP4Read(FILE_1) as file_handle:
    print(f'File handle valid is {file_handle.is_valid()}')
    filename = pymp4v2.raw.MP4GetFilename(file_handle)
    print(filename)

print(f'File handle valid is {file_handle.is_valid()}')

print('=== End tests')