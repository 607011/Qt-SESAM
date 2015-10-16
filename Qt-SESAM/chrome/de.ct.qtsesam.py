#!/usr/bin/env python

import struct
import sys

if sys.platform == "win32":
  import os, msvcrt
  msvcrt.setmode(sys.stdin.fileno(), os.O_BINARY)
  msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)


def send_message(message):
  sys.stdout.write(struct.pack('I', len(message)))
  sys.stdout.write(message)
  sys.stdout.flush()


def main():
  while True:
    text_length_bytes = sys.stdin.read(4)
    text_length = struct.unpack('i', text_length_bytes)[0]
    text = sys.stdin.read(text_length)
    send_message('{"echo": %s}' % text)


if __name__ == '__main__':
  main()
