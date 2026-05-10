#!/usr/bin/env python3
"""
Grow memory in fixed chunks until a target or until the process is stopped (OOM / job limit).

Good for watching when a HAPP job RAM cap bites as allocation increases.

Example:
  happ simulate --ram 3 --session
  python alloc_stepped_grow.py --chunk-mib 128 --max-mib 4096 --pause 0.5
"""
from __future__ import annotations

import argparse
import sys
import time


def main() -> None:
    p = argparse.ArgumentParser(description="Allocate RAM in growing chunks.")
    p.add_argument("--chunk-mib", type=float, default=64.0, help="Each step adds this many MiB.")
    p.add_argument("--max-mib", type=float, default=8192.0, help="Stop after this total MiB (approx).")
    p.add_argument("--pause", type=float, default=0.25, help="Seconds between chunks.")
    p.add_argument(
        "--touch-chunks",
        action="store_true",
        help="Touch each chunk after alloc to encourage commit.",
    )
    args = p.parse_args()

    chunk = int(args.chunk_mib * 1024 * 1024)
    max_bytes = int(args.max_mib * 1024 * 1024)
    if chunk <= 0 or max_bytes <= 0:
        print("chunk-mib and max-mib must be positive.", file=sys.stderr)
        sys.exit(1)

    chunks: list[bytearray] = []
    total = 0
    print(f"Growing by {args.chunk_mib} MiB steps up to ~{args.max_mib} MiB.", flush=True)
    try:
        while total < max_bytes:
            b = bytearray(chunk)
            if args.touch_chunks:
                step = 64 * 1024
                for i in range(0, len(b), step):
                    b[i] = 1
                if len(b) > 0:
                    b[-1] = 1
            chunks.append(b)
            total += len(b)
            print(f"  total ~{total / (1024 * 1024):.1f} MiB", flush=True)
            time.sleep(args.pause)
    except MemoryError as e:
        print(f"MemoryError at ~{total / (1024 * 1024):.1f} MiB: {e}", file=sys.stderr)
        sys.exit(2)

    print("Reached max-mib without failure.", flush=True)


if __name__ == "__main__":
    main()
