#!/usr/bin/env python3
"""Generator of canonical FRAMER_PCS_TX test vectors (.hex, Intel HEX).

The format follows the reference file reference_vec/FRAMER_PCS_TX.hex:
  * 46-byte header @ 0x0C000000, fields are big-endian (MSB first on the wire);
  * signals_info = 0x0303, widths start at offset 0x22 with NO padding:
      input:  0x0140 (320) 0x0008 (8) 0x0004 (4)  -> 40+1+1 = 42 bytes;
      output: 0x006F (111) 0x0008 (8) 0x0005 (5)  -> 14+1+1 = 16 bytes;
  * total_data_length = N * (42 + 16);
  * input vectors @ input_base (default 0x01000000), 42 bytes each:
      [0..39] OTL4_10_DATA (320 bits), [40] TXOHIDAT, [41] flags in bits 7-4;
  * output vectors @ output_base (default 0x00000000), 16 bytes each:
      [0..13] DATA_OUT (111 bits), [14] TXOHEDAT, [15] flags in bits 7-3.

Usage:
  tools/gen_test_vector.py --count 4 --out test_vectors/test_canonical_4vec.hex \
      --pattern random --seed 42
"""
import argparse
import random
import sys

SIG_TEST = 0x54455354          # "TEST"
MODULES = {"FRTX": 0x46525458, "OTU4": 0x4F545534, "SCRM": 0x5343524D}
RESERVED = 0xC0FEF00D
IN_SIZE = 42
OUT_SIZE = 16
IN_WIDTHS = (0x0140, 0x0008, 0x0004)    # 320, 8, 4
OUT_WIDTHS = (0x006F, 0x0008, 0x0005)   # 111, 8, 5
IN_FLAG_NIBBLE_IDLE = 0xC        # RATE_CONTROL=1, nHARD_RESET=1 (reset inactive), byte = 0xC0
IN_FLAG_NIBBLE_RESET = 0x0       # all flags 0 -> nHARD_RESET=0 (reset asserted)


def build_header(count, module, in_base, out_base):
    h = b""
    h += SIG_TEST.to_bytes(4, "big")
    h += MODULES[module].to_bytes(4, "big")
    h += (count * (IN_SIZE + OUT_SIZE)).to_bytes(4, "big")
    h += in_base.to_bytes(4, "big")
    h += IN_SIZE.to_bytes(4, "big")
    h += out_base.to_bytes(4, "big")
    h += OUT_SIZE.to_bytes(4, "big")
    h += RESERVED.to_bytes(4, "big")
    h += bytes([3, 3])
    for w in IN_WIDTHS + OUT_WIDTHS:
        h += w.to_bytes(2, "big")
    assert len(h) == 46, len(h)
    return h


def gen_input_vector(i, pattern, rng):
    if pattern == "zeros":
        data = bytes(IN_SIZE)
        flags = IN_FLAG_NIBBLE_RESET << 4
        return data[:41] + bytes([flags])
    if pattern == "inc":
        body = bytes((i * IN_SIZE + j) & 0xFF for j in range(41))
    else:
        body = bytes(rng.randrange(256) for _ in range(41))
    flags = IN_FLAG_NIBBLE_IDLE << 4 if (i % 4) else IN_FLAG_NIBBLE_RESET << 4
    return body + bytes([flags])


def gen_output_vector(i, pattern, rng):
    if pattern == "zeros":
        return bytes(OUT_SIZE)
    if pattern == "inc":
        body = bytes((0xC0 + i * OUT_SIZE + j) & 0xFF for j in range(15))
    else:
        body = bytes(rng.randrange(256) for _ in range(15))
    flags5 = (i * 5 + 3) & 0x1F           # assorted 5-bit combinations
    return body + bytes([flags5 << 3])


def rec(rtype, offset, data):
    b = bytes([len(data), (offset >> 8) & 0xFF, offset & 0xFF, rtype]) + data
    return ":" + b.hex().upper() + "%02X" % ((-sum(b)) & 0xFF)


def ela(upper16):
    return rec(0x04, 0, upper16.to_bytes(2, "big"))


def emit_region(lines, base, blob, rec_len):
    lines.append(ela(base >> 16))
    addr = base & 0xFFFF
    for off in range(0, len(blob), rec_len):
        chunk = blob[off:off + rec_len]
        while addr + len(chunk) > 0x10000:          # crossing a 64K boundary
            split = 0x10000 - addr
            lines.append(rec(0, addr, chunk[:split]))
            base += split
            lines.append(ela(base >> 16))
            addr = base & 0xFFFF
            chunk = chunk[split:]
        lines.append(rec(0, addr, chunk))
        addr += len(chunk)
        if addr >= 0x10000:
            base += 0x10000
            lines.append(ela(base >> 16))
            addr -= 0x10000


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument("--count", type=int, required=True, help="number of vectors N")
    p.add_argument("--out", required=True, help="output .hex file")
    p.add_argument("--pattern", choices=("random", "zeros", "inc"), default="random")
    p.add_argument("--seed", type=int, default=0)
    p.add_argument("--module", choices=sorted(MODULES), default="FRTX")
    p.add_argument("--in-base", type=lambda x: int(x, 0), default=0x01000000)
    p.add_argument("--out-base", type=lambda x: int(x, 0), default=0x00000000)
    p.add_argument("--rec-len", type=int, default=32, help="bytes per data record")
    args = p.parse_args()
    if args.count <= 0:
        sys.exit("count must be positive")

    rng = random.Random(args.seed)
    lines = []
    emit_region(lines, 0x0C000000,
                build_header(args.count, args.module, args.in_base, args.out_base),
                args.rec_len)
    out_blob = b"".join(gen_output_vector(i, args.pattern, rng)
                        for i in range(args.count))
    emit_region(lines, args.out_base, out_blob, args.rec_len)
    in_blob = b"".join(gen_input_vector(i, args.pattern, rng)
                       for i in range(args.count))
    emit_region(lines, args.in_base, in_blob, args.rec_len)
    lines.append(":00000001FF")

    with open(args.out, "w") as f:
        f.write("\n".join(lines) + "\n")
    print("wrote %s: N=%d, %d lines, in=0x%08X out=0x%08X total=%d"
          % (args.out, args.count, len(lines), args.in_base, args.out_base,
             args.count * (IN_SIZE + OUT_SIZE)))


if __name__ == "__main__":
    main()
