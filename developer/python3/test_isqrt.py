#!/usr/bin/env python3

import sys
import random
from isqrt import isqrt

def test_isqrt(num_tests, max_bits):
  failed = 0
  passed = 0

  for i in range(1, num_tests + 1):
    bits = random.randint(1, max_bits)
    n = random.getrandbits(bits)

    root = isqrt(n)
    lower = root * root
    upper = (root + 1) * (root + 1)

    if lower <= n < upper:
      passed += 1
    else:
      print(f"\n❌ Test failed:")
      print(f"bits   = {bits}")
      print(f"n      = {hex(n)}")
      print(f"root   = {hex(root)}")
      if lower > n:
        print("Reason = root is too large")
      elif upper <= n:
        print("Reason = root + 1 is too small")
      failed += 1

    if i % 1000 == 0:
      print(".", end="", flush=True)

  if num_tests >= 10:
    print()

  print(f"✅ Passed: {passed}")
  print(f"❌ Failed: {failed}")

def print_root(n):
  print_bin(isqrt(n))

def reduce(n):
  n  

def main():
  if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} <n_tests> <max_bits>")
    sys.exit(1)

  try:
    n = int(sys.argv[1], 0)
    max = int(sys.argv[2], 0)
  except ValueError:
    print("Error: argument must be a valid integer.")
    sys.exit(1)

  if n < 0:
    print("Error: number of tests must be non-negative.")
    sys.exit(1)

  if max < 4:
    print("Error: max_bits must be at least 4.")
    sys.exit(1)

  test_isqrt(n, max)

if __name__ == "__main__":
  main()
