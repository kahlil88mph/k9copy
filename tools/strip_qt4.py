#!/usr/bin/env python3
"""Resolve `#if QT_VERSION >= 0x050000` blocks as true: keep the Qt5 branch,
drop the `#else` (Qt4) branch. Other preprocessor blocks are left untouched."""
import re
import sys

COND = re.compile(r'^\s*#\s*if\s+QT_VERSION\s*>=\s*0?[xX]?0?50000\b')
IF = re.compile(r'^\s*#\s*if')
ELSE = re.compile(r'^\s*#\s*else\b')
ELIF = re.compile(r'^\s*#\s*elif\b')
ENDIF = re.compile(r'^\s*#\s*endif\b')


def process(lines):
    out = []
    # stack entries: None for unrelated #if, or 'keep'/'drop' state for ours
    stack = []
    for line in lines:
        dropping = any(s == 'drop' for s in stack)
        if COND.match(line):
            stack.append('keep')
            continue
        if IF.match(line):
            stack.append(None)
            if not dropping:
                out.append(line)
            continue
        if stack and stack[-1] is not None and (ELSE.match(line) or ELIF.match(line)):
            if ELIF.match(line):
                raise SystemExit('unexpected #elif in QT_VERSION block')
            stack[-1] = 'drop'
            continue
        if ENDIF.match(line):
            top = stack.pop()
            if top is None and not dropping:
                out.append(line)
            continue
        if not dropping:
            out.append(line)
    if stack:
        raise SystemExit('unbalanced preprocessor blocks')
    return out


for path in sys.argv[1:]:
    with open(path, encoding='utf-8', errors='surrogateescape') as f:
        lines = f.readlines()
    new = process(lines)
    if new != lines:
        with open(path, 'w', encoding='utf-8', errors='surrogateescape') as f:
            f.writelines(new)
        print(f'{path}: {len(lines) - len(new)} lines removed')
