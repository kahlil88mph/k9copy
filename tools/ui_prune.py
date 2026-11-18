#!/usr/bin/env python3
"""Remove named widgets (and the layout <item> holding them), plus signal/slot
connections and slot declarations that refer to them, from a Qt .ui file.
usage: ui_prune.py file.ui --widgets a,b --slots s1,s2"""
import argparse
import xml.etree.ElementTree as ET

ap = argparse.ArgumentParser()
ap.add_argument('ui')
ap.add_argument('--widgets', default='')
ap.add_argument('--slots', default='')
a = ap.parse_args()
widgets = set(filter(None, a.widgets.split(',')))
slots = set(filter(None, a.slots.split(',')))

tree = ET.parse(a.ui)
root = tree.getroot()
removed = []
for parent in list(root.iter()):
    for child in list(parent):
        target = child
        if child.tag == 'item':
            w = child.find('widget')
            target = w if w is not None else child
        if target.tag == 'widget' and target.get('name') in widgets:
            parent.remove(child)
            removed.append(target.get('name'))
conns = root.find('connections')
if conns is not None:
    for c in list(conns):
        if (c.findtext('sender') in widgets or c.findtext('receiver') in widgets
                or (c.findtext('slot') or '').split('(')[0] in slots):
            conns.remove(c)
sl = root.find('slots')
if sl is not None:
    for s in list(sl):
        if (s.text or '').split('(')[0] in slots:
            sl.remove(s)
tree.write(a.ui, encoding='UTF-8', xml_declaration=True)
print(a.ui, 'removed:', ', '.join(sorted(set(removed))))
