#!/usr/bin/env python3
"""Where the wall time of a ninja build went, read from its .ninja_log.

Usage: ninjalog.py <.ninja_log> [--top N] [--bucket SECONDS] [--trace FILE.json]

ninja records when every step started and ended, in milliseconds from the start of the build. The report gives the
longest steps, the time by kind of step, how many steps ran at a time over the build, and what was still running
near the end: a build that ends on one long-running step was waiting for it, one that ends on many short ones was
working. --trace writes the steps as a Chrome trace, one row per concurrently running step, for ui.perfetto.dev or
chrome://tracing.
"""

import argparse
import json
import sys


def read_log(path):
    steps = {}
    with open(path, encoding='utf-8', errors='replace') as log:
        for line in log:
            if line.startswith('#'):
                continue
            fields = line.rstrip('\n').split('\t')
            if len(fields) < 5:
                continue
            start, end, output, digest = int(fields[0]), int(fields[1]), fields[3], fields[4]
            # a step with several outputs is logged once per output
            steps.setdefault((start, end, digest), output)
    return sorted((start / 1000, end / 1000, output) for (start, end, _), output in steps.items())


def kind(output):
    name = output.rsplit('/', 1)[-1]
    if name.endswith('.o'):
        return 'compile'
    if '_autogen' in output:
        return 'autogen'
    if name.endswith('.qm'):
        return 'lrelease'
    if '.so' in name or name.endswith('.a') or '.' not in name:
        return 'link'
    return 'other'


def active_at(steps, moment):
    return [step for step in steps if step[0] <= moment < step[1]]


def concurrency(steps, wall, bucket):
    buckets = [0.0] * (int(wall / bucket) + 1)
    for start, end, _ in steps:
        first, last = int(start / bucket), int(end / bucket)
        for b in range(first, min(last, len(buckets) - 1) + 1):
            low, high = max(start, b * bucket), min(end, (b + 1) * bucket)
            buckets[b] += max(0.0, high - low)
    return [busy / bucket for busy in buckets]


def report(steps, top, bucket):
    wall = max(end for _, end, _ in steps) - min(start for start, _, _ in steps)
    total = sum(end - start for start, end, _ in steps)
    print(f'{len(steps)} steps, wall {wall:.1f} s, step time {total:.1f} s, '
          f'{total / wall:.1f} steps running on average')

    by_kind = {}
    for start, end, output in steps:
        seconds, count = by_kind.get(kind(output), (0.0, 0))
        by_kind[kind(output)] = (seconds + end - start, count + 1)
    print('by kind: ' + ', '.join(f'{k} {seconds:.0f} s in {count}' for k, (seconds, count)
                                  in sorted(by_kind.items(), key=lambda item: -item[1][0])))

    print(f'\nlongest {top} steps:')
    for start, end, output in sorted(steps, key=lambda step: step[0] - step[1])[:top]:
        print(f'  {end - start:6.1f} s  {start:6.1f} -> {end:6.1f}  {output}')

    print(f'\nsteps running at a time, per {bucket:g} s:')
    print('  ' + ' '.join(f'{n:.0f}' for n in concurrency(steps, wall, bucket)))

    tail = wall * 0.9
    print(f'\nrunning at {tail:.1f} s, nine tenths into the build:')
    for start, end, output in sorted(active_at(steps, tail), key=lambda step: step[0]):
        print(f'  since {start:6.1f} s, until {end:6.1f} s  {output}')
    last = max(steps, key=lambda step: step[1])
    print(f'\nlast to finish: {last[2]} ({last[0]:.1f} -> {last[1]:.1f} s)')


def write_trace(steps, path):
    lanes = []
    events = []
    for start, end, output in steps:
        lane = next((i for i, busy_until in enumerate(lanes) if busy_until <= start), None)
        if lane is None:
            lane = len(lanes)
            lanes.append(0)
        lanes[lane] = end
        events.append({'name': output, 'cat': kind(output), 'ph': 'X', 'pid': 1, 'tid': lane,
                       'ts': int(start * 1e6), 'dur': int((end - start) * 1e6)})
    with open(path, 'w', encoding='utf-8') as out:
        json.dump({'traceEvents': events}, out)
    print(f'\ntrace with {len(events)} steps on {len(lanes)} rows written to {path}')


def main():
    parser = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    parser.add_argument('log', help='the .ninja_log of a build')
    parser.add_argument('--top', type=int, default=15, help='how many of the longest steps to list')
    parser.add_argument('--bucket', type=float, default=5, help='seconds per column of the concurrency line')
    parser.add_argument('--trace', metavar='FILE.json', help='also write a Chrome trace of the steps')
    args = parser.parse_args()
    steps = read_log(args.log)
    if not steps:
        sys.exit(f'{args.log}: no steps recorded')
    report(steps, args.top, args.bucket)
    if args.trace:
        write_trace(steps, args.trace)


if __name__ == '__main__':
    main()
