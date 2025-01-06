#!/usr/bin/env python3
import os
import sys
import cgitb
import urllib

cgitb.enable()

with open("./tools/filepos", "cr") as pos_file:
    position: int = int(pos_file.read().strip())
filename: str = os.environ.get("FILENAME")

filepath: str = "./site/downloads/" + filename

print("HTTP/1.1 200 OK")
print("Content-Type: application/octet-stream")  # Force le téléchargement
print(f"Content-Disposition: attachment; filename=\"{filename}\"")
filesize = os.path.getsize(filepath)
print(f"Content-Length: {filesize}")
print()

with open(filepath, "rb") as f:
	sys.stdout.write(f.read().decode('latin1'))
    # chunk_size: int = 8192
    # chunk: str
    # while chunk := filepath.read(chunk_size):
    #     sys.stdout.buffer.write(chunk)
    #     sys.stdout.buffer.flush()