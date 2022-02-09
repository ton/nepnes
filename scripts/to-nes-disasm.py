#!/bin/env python
import fileinput
import re

for line in fileinput.input():
    match = re.match('^\$(.*): (.*) \((.*)\)$', line)
    if match:
        address = match.group(1)
        assembly = match.group(2).strip()
        encoding = match.group(3)

        if len(assembly) == 3:
            assembly += ' '

        print('$%s\t%s%s\t%s' % (address, encoding, ' ' * (4 - (len(encoding) // 2) - (0 if assembly else 1)), assembly if assembly else '.byte $' + encoding))
