#!/usr/bin/env python3
"""
Hold one large contiguous allocation (committed RAM) for a duration.

Use with a HAPP session to see job memory limits, e.g.:
  happ simulate --ram 2 --session
  python alloc_hold.py --gib 1.5 --seconds 120

Exceeding --ram (job cap) should fail or terminate once committed memory passes the limit.
"""
from __future__ import annotations

import argparse
import sys
import time
from typing import Optional


def _bytes_from_mib_gib(mib: Optional[float], gib: Optional[float]) -> int:
    if mib is not None and gib is not None:
        raise SystemExit("Use either --mib or --gib, not both.")
    if mib is not None:
        return int(mib * 1024 * 1024)
    if gib is not None:
        return int(gib * 1024 * 1024 * 1024)
    raise SystemExit("Specify --mib or --gib.")


def main() -> None:
    p = argparse.ArgumentParser(description="Allocate and hold RAM (bytearray).")
    p.add_argument("--mib", type=float, default=None, help="Size in mebibytes (MiB).")
    p.add_argument("--gib", type=float, default=None, help="Size in gibibytes (GiB).")
    p.add_argument(
        "--seconds",
        type=float,
        default=None,
        help="How long to hold memory (default: run until Ctrl+C).",
    )
    p.add_argument(
        "--touch",
        action="store_true",
        help="Write every 64 KiB page so the OS actually commits physical/RAM backing.",
    )
    args = p.parse_args()

    n = _bytes_from_mib_gib(args.mib, args.gib)
    print(f"Allocating {n / (1024 * 1024):.2f} MiB...", flush=True)
    try:
        buf = bytearray(n)
    except MemoryError as e:
        print(f"MemoryError: {e}", file=sys.stderr)
        sys.exit(1)

    if args.touch:
        step = 64 * 1024
        for i in range(0, n, step):
            buf[i] = 1
        if n > 0:
            buf[-1] = 1
    else:
        buf[0] = 1
        if n > 1:
            buf[-1] = 1

    if args.seconds is None:
        print("Allocation held. Press Ctrl+C to stop.", flush=True)
    else:
        print(
            f"Allocation held for {args.seconds} s (Ctrl+C stops early).",
            flush=True,
        )
    try:
        if args.seconds is None:
            while True:
                time.sleep(3600)
        else:
            time.sleep(args.seconds)
    except KeyboardInterrupt:
        print("\nStopped (interrupted).", flush=True)
    else:
        if args.seconds is not None:
            print("Done (held for requested duration).", flush=True)


if __name__ == "__main__":
    main()
