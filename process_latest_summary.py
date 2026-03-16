#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path
from statistics import mean


DEFAULT_SUMMARY = Path("stride-logs/latest/summary.json")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Process a STRIDE summary.json file in JSON Lines format."
    )
    parser.add_argument(
        "summary_path",
        nargs="?",
        default=DEFAULT_SUMMARY,
        type=Path,
        help="Path to the summary.json file (default: stride-logs/latest/summary.json)",
    )
    parser.add_argument(
        "--top-slowest",
        type=int,
        default=5,
        help="How many slowest instances to print (default: 5)",
    )
    return parser.parse_args()


def load_entries(summary_path: Path) -> list[dict]:
    if not summary_path.is_file():
        raise FileNotFoundError(f"Summary file not found: {summary_path}")

    entries: list[dict] = []
    with summary_path.open("r", encoding="utf-8") as handle:
        for line_number, raw_line in enumerate(handle, start=1):
            line = raw_line.strip()
            if not line:
                continue
            try:
                entries.append(json.loads(line))
            except json.JSONDecodeError as exc:
                raise ValueError(
                    f"Invalid JSON at line {line_number} in {summary_path}: {exc}"
                ) from exc

    if not entries:
        raise ValueError(f"No entries found in {summary_path}")

    return entries


def fmt_float(value: float) -> str:
    return f"{value:.3f}"


def print_report(summary_path: Path, entries: list[dict], top_slowest: int) -> None:
    result_counts = Counter(entry.get("s_result", "UNKNOWN") for entry in entries)

    valid_entries = [entry for entry in entries if entry.get("s_result") == "Valid"]
    improved = [
        entry for entry in valid_entries if entry.get("s_score", 0) < entry.get("s_prev_best", 0)
    ]
    matched = [
        entry for entry in valid_entries if entry.get("s_score", 0) == entry.get("s_prev_best", 0)
    ]
    worse = [
        entry for entry in valid_entries if entry.get("s_score", 0) > entry.get("s_prev_best", 0)
    ]

    wall_times = [entry.get("s_wtime", 0.0) for entry in entries]
    cpu_times = [entry.get("s_utime", 0.0) + entry.get("s_stime", 0.0) for entry in entries]
    maxrss_values = [entry.get("s_maxrss", 0) / (1024 * 1024) for entry in entries]
    heuristic_scores = [entry.get("s_heuristic_score", 0.0) for entry in entries]

    print(f"File: {summary_path}")
    print(f"Entries: {len(entries)}")
    print("Results:")
    for result, count in sorted(result_counts.items()):
        print(f"  {result}: {count}")

    print("Quality:")
    print(f"  Improved vs prev_best: {len(improved)}")
    print(f"  Matched prev_best: {len(matched)}")
    print(f"  Worse than prev_best: {len(worse)}")
    if valid_entries:
        valid_scores = [entry.get("s_score", 0) for entry in valid_entries]
        prev_best_scores = [entry.get("s_prev_best", 0) for entry in valid_entries]
        print(f"  Avg score: {fmt_float(mean(valid_scores))}")
        print(f"  Avg prev_best: {fmt_float(mean(prev_best_scores))}")
        print(f"  Avg heuristic score: {fmt_float(mean(heuristic_scores))}")

    print("Resources:")
    print(f"  Avg wall time: {fmt_float(mean(wall_times))} s")
    print(f"  Max wall time: {fmt_float(max(wall_times))} s")
    print(f"  Avg CPU time: {fmt_float(mean(cpu_times))} s")
    print(f"  Avg max RSS: {fmt_float(mean(maxrss_values))} MiB")
    print(f"  Max max RSS: {fmt_float(max(maxrss_values))} MiB")

    slowest = sorted(entries, key=lambda entry: entry.get("s_wtime", 0.0), reverse=True)
    if top_slowest > 0:
        print(f"Slowest {min(top_slowest, len(slowest))}:")
        for entry in slowest[:top_slowest]:
            print(
                "  "
                f"{entry.get('s_key', 'UNKNOWN')} "
                f"result={entry.get('s_result', 'UNKNOWN')} "
                f"score={entry.get('s_score', 'NA')} "
                f"prev_best={entry.get('s_prev_best', 'NA')} "
                f"wtime={fmt_float(entry.get('s_wtime', 0.0))} s "
                f"rss={fmt_float(entry.get('s_maxrss', 0) / (1024 * 1024))} MiB"
            )


if __name__ == "__main__":
    args = parse_args()
    entries = load_entries(args.summary_path)
    print_report(args.summary_path, entries, args.top_slowest)
