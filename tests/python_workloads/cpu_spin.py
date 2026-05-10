#!/usr/bin/env python3
"""
CPU-bound loop to stress the job CPU rate cap (not RAM).

Example:
  happ simulate --cpu 1 --session
  python cpu_spin.py --seconds 30

Optional small allocation to add a bit of memory traffic.
"""
from __future__ import annotations

import argparse
import hashlib
import time
from typing import Optional


def main() -> None:
    p = argparse.ArgumentParser(description="CPU-heavy busy loop.")
    p.add_argument("--seconds", type=float, default=30.0, help="Run time.")
    p.add_argument(
        "--alloc-mib",
        type=float,
        default=0.0,
        help="Also hold this many MiB (optional mixed workload).",
    )
    args = p.parse_args()

    hold: Optional[bytearray] = None
    if args.alloc_mib > 0:
        n = int(args.alloc_mib * 1024 * 1024)
        print(f"Holding {args.alloc_mib} MiB while spinning CPU...", flush=True)
        hold = bytearray(n)
        hold[0] = 1
        hold[-1] = 1

    deadline = time.time() + args.seconds
    iters = 0
    x = b"happ-cpu-load"
    print(f"CPU spin for {args.seconds} s...", flush=True)
    while time.time() < deadline:
        # Mix integer and crypto work so the interpreter stays busy.
        x = hashlib.sha256(x).digest()
        iters += 1
    print(f"Done. Hash iterations: {iters}", flush=True)
    del hold


if __name__ == "__main__":
    main()
