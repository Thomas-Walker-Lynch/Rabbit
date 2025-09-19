
import sys
import random
from iroot import iroot

def test_reduce(num_tests, max_bits):
  failed = 0
  passed = 0

  hex_width = (max_bits + 3) // 4       # ceil(max_bits / 4) for hex digit count
  hex_widtho2 = (max_bits + 7) // 8     # ceil(max_bits / 8) for reduced-width fields

  field = f"#0{hex_width + 2}x"
  fieldo2 = f"#0{hex_widtho2 + 2}x"

  header = (
    f"{'z':>{hex_width + 2}}  "
    f"{'n0h':>{hex_widtho2 + 2}}  "
    f"{'r_h':>{hex_widtho2 + 2}}  "
    f"{'r_hp':>{hex_widtho2 + 2}}"
  )
  print(header)
  print("-" * len(header))

  for i in range(1, num_tests + 1):
    bits = random.randint(1, max_bits)
    z = random.getrandbits(bits)

    n0h, r_h, r_hp = iroot(z)

    # Format fields, fallback to full-width hex if overflow detected
    z_str     = format(z, field)     if z.bit_length() <= max_bits else f"{z:#x}"
    n0h_str   = format(n0h, fieldo2) if n0h.bit_length() <= max_bits else f"{n0h:#x}"
    r_h_str  = format(r_h, fieldo2) if r_h.bit_length() <= max_bits else f"{r_h:#x}"
    r_hp_str = format(r_hp, fieldo2) if r_hp.bit_length() <= max_bits else f"{r_hp:#x}"

    print(f"{z_str}  {n0h_str}  {r_h_str}  {r_hp_str}")

    if i % 1000 == 0:
      print(".", end="", flush=True)

  if num_tests >= 1000:
    print()



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

  test_reduce(n, max)

if __name__ == "__main__":
  main()
