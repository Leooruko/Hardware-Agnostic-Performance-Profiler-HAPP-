#!/usr/bin/env python3
"""
Many separate Python objects (lists of ints) to inflate RAM with interpreter overhead.

Uses more memory per "useful" byte than alloc_hold.py; useful for messy / fragmented patterns.

Example:
  happ simulate --ram 2 --session
  python many_objects.py --target-mib 400 --chunk-size 10000
"""
from __future__ import annotations

import argparse
import sys
import time


def main() -> None:
    p = argparse.ArgumentParser(description="Many medium-sized Python lists (RAM overhead).")
    p.add_argument("--target-mib", type=float, required=True, help="Approximate goal in MiB.")
    p.add_argument(
        "--chunk-size",
        type=int,
        default=50_000,
        help="List length per chunk (larger = fewer objects, less overhead).",
    )
    args = p.parse_args()

    target = int(args.target_mib * 1024 * 1024)
    if target <= 0 or args.chunk_size <= 0:
        print("target-mib and chunk-size must be positive.", file=sys.stderr)
        sys.exit(1)

    chunks: list[list[int]] = []
    # Rough heuristic: each int ~28 bytes on 64-bit CPython; lists add overhead.
    approx_per_elem = 28
    bytes_per_chunk = args.chunk_size * approx_per_elem
    total_est = 0
    print(
        f"Growing lists toward ~{args.target_mib} MiB (chunk {args.chunk_size}, ~{bytes_per_chunk} B/chunk)...",
        flush=True,
    )
    try:
        while total_est < target:
            chunks.append(list(range(args.chunk_size)))
            total_est += bytes_per_chunk
            if len(chunks) % 10 == 0:
                print(f"  chunks={len(chunks)} ~est {total_est / (1024 * 1024):.1f} MiB", flush=True)
    except MemoryError as e:
        print(f"MemoryError after ~{len(chunks)} chunks: {e}", file=sys.stderr)
        sys.exit(2)

    print(f"Stopped at ~{total_est / (1024 * 1024):.1f} MiB (estimate). Holding; Ctrl+C to exit.", flush=True)
    try:
        while True:
            time.sleep(3600)
    except KeyboardInterrupt:
        print("\nStopped.", flush=True)


if __name__ == "__main__":
    main()
