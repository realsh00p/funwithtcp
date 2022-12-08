#!/usr/bin/env python3

import argparse
import socket
import sys

PORT=20000
BUF_SIZE=8192

def main():

  parser = argparse.ArgumentParser()
  parser.add_argument('-u', '--udp', action='store_true')
  parser.add_argument('-t', '--tcp', action='store_true')
  parser.add_argument('filename')
  args = parser.parse_args()


  with open(args.filename, 'rb') as f:
    contents = f.read()

  print(f"file: {args.filename} size: {len(contents)}")

  if args.udp:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    send = lambda sock, chunk : sock.sendto(chunk, ('localhost', PORT))
  elif args.tcp:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', PORT))
    send = lambda sock, chunk : sock.sendall(chunk)
  else:
    print("require either udp or tcp")
    sys.exit(1)


  while len(contents) > 0:
    if len(contents) > BUF_SIZE:
      chunk = contents[0:BUF_SIZE]
    else:
      chunk = contents
    contents = contents[BUF_SIZE:]
    send(sock, chunk)

  sock.close()

if __name__ == "__main__":
    main()
