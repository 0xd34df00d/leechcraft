#!/usr/bin/env python3
"""The part of a failed cell's log that says why it failed, to quote in the build's mail.

Usage: quotefailure.py <runci.sh exit status> <log> [--strip PREFIX]

What is quoted depends on what failed, as tools/docker/runci.sh reports it: for a failed build, ninja's report of every
failed step, minus the command line; for a failed configure, CMake's errors; for failed tests, QtTest's failures and ctest's
list of the failed tests; for anything else, or when none of these turns up, the end of the log. --strip removes PREFIX
wherever it occurs, so that paths read relative to the checkout.
"""

import argparse
import re
import sys

MAX_BLOCKS = 10
MAX_BLOCK_LINES = 30
TAIL_LINES = 20

NINJA_PROGRESS = re.compile(r'^\[\d+/\d+\] ')
CMAKE_MESSAGE = re.compile(r'^CMake (Error|Warning|Deprecation Warning)\b')


def capped(block):
    while block and not block[-1].strip():
        block.pop()
    if len(block) > MAX_BLOCK_LINES:
        block = block[:MAX_BLOCK_LINES] + [f'[{len(block) - MAX_BLOCK_LINES} more lines]']
    return block


def failed_steps(lines):
    blocks = []
    block = None
    skip_command = False
    for line in lines:
        if line.startswith('FAILED: '):
            block = [line]
            blocks.append(block)
            # ninja prints the step's command line right after, a few kilobytes of flags
            skip_command = True
        elif block is not None and (NINJA_PROGRESS.match(line) or line.startswith('ninja: ')):
            block = None
        elif block is not None:
            if not skip_command:
                block.append(line)
            skip_command = False
    return blocks


def cmake_errors(lines):
    blocks = []
    block = None
    for line in lines:
        if line.startswith('CMake Error'):
            block = [line]
            blocks.append(block)
        elif block is not None and (line.startswith('-- ') or CMAKE_MESSAGE.match(line)):
            block = None
        elif block is not None:
            block.append(line)
    return blocks


def test_failures(lines):
    blocks = []
    block = None
    for line in lines:
        if line.startswith(('FAIL!  :', 'QFATAL :', 'XPASS  :')):
            block = [line]
            blocks.append(block)
        elif line.startswith('The following tests FAILED:'):
            block = [line]
            # the list of failed tests says the most when the failures above it are many
            blocks.insert(0, block)
        elif block is not None and line[:1] in (' ', '\t') and line.strip():
            block.append(line)
        else:
            block = None
    return blocks


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('status', type=int)
    parser.add_argument('log')
    parser.add_argument('--strip', default='')
    args = parser.parse_args()

    with open(args.log, encoding='utf-8', errors='replace') as log:
        lines = log.read().splitlines()
    if args.strip:
        lines = [line.replace(args.strip, '') for line in lines]

    extract = {1: failed_steps, 2: cmake_errors, 3: test_failures}.get(args.status)
    blocks = [capped(block) for block in extract(lines)] if extract else []
    blocks = [block for block in blocks if block]
    if not blocks:
        print('\n'.join(lines[-TAIL_LINES:]))
        return
    if len(blocks) > MAX_BLOCKS:
        blocks = blocks[:MAX_BLOCKS] + [[f'and {len(blocks) - MAX_BLOCKS} more']]
    print('\n\n'.join('\n'.join(block) for block in blocks))


if __name__ == '__main__':
    sys.exit(main())
