#!/usr/bin/env python3

import cgi
import os
import cgitb
import sys
from typing import Optional

cgitb.enable()

directory: str = "./site/downloads/"

raw_input: str = sys.stdin.read()

index_start: int = raw_input.find("filename=") + 10
index_end: int = raw_input.find("\"", index_start)

filename: str = raw_input[index_start:index_end]

directory += filename

start: int = raw_input.find("\r\n\r\n")
start = raw_input.find("Content-Type: ", start)
start = raw_input.find("\r\n", start) + 4
end: int = raw_input.find(os.environ.get("BOUNDARY") + "--") - 4

with open(directory, "wb") as f:
	f.write(raw_input[start:end])

print("You successfully uploaded " + filename)