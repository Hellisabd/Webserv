#!/usr/bin/python3

import os

text = os.environ.get('text')
word_count = len(text.split())
print(word_count)