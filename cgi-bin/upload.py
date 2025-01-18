#!/usr/bin/env python3

import cgi
import os
import cgitb
import sys
from typing import Optional

while 1:
	i = 1

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

with open(directory, "w") as f:
	f.write(raw_input[start:end])

content: str = f"""<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Bienvenue sur webserv</title>
    <style>
        body {{
            a {{
                display: inline-block;
                margin-top: 1em;
                padding: 0.5em 1em;
                background-color: #3498db;
                color: white;
                text-decoration: none;
                border-radius: 4px;
                font-size: 1em;
            }}
        }}
    </style>
</head>
<body> 
	You successfully uploaded {filename}
	<br>
	<a href=\"/\">HOME</a>
</body>
</html>"""

print(content)