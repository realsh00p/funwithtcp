#!/usr/bin/env python3

import socket
import sys

PORT=20000

def main():
  sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

  server_address = ('localhost', PORT)
  sock.connect(server_address)

  try:
    sock.sendall(bytes([0] * 1024))
  finally:
    sock.close()

if __name__ == "__main__":
    main()
