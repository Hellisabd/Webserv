#!/usr/bin/env python3

import cgi
import os
import cgitb
import sys
import urllib.parse
from typing import Optional

cgitb.enable()

directory: str = "./site/downloads/"

raw_input: str = sys.stdin.read()

body_start: int = raw_input.find("\r\n\r\n") + 4

body: str = raw_input[body_start:]

parsed_body: str = urllib.parse.parse_qs(body)

filename = parsed_body.get("filename", [""])[0]
content = parsed_body.get("content", [""])[0]

filename += ".txt"
directory += filename

with open(directory, "w") as f:
	f.write(content)

content = f"""<!DOCTYPE html>
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
	You successfully created {filename}
	<br>
	<a href=\"/\">HOME</a>
</body>
</html>"""

print(content)