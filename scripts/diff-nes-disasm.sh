#!/bin/sh
build/da -i "$1" | scripts/to-nes-disasm.py | grep -v .byte > /tmp/a
#unzip -p "$1" > /tmp/rom
~/dev/external/nes-disasm/nes-disasm "$1" | grep -v .byte | grep -ve '^;' | grep -ve '^$' > /tmp/b
sed -i 's/; \$.... //' /tmp/b
if ! diff -q /tmp/a /tmp/b > /dev/null; then
    nvim -d /tmp/a /tmp/b
    exit_code=1
else
    echo "No differences found for $1..."
    exit_code=0
fi
# rm /tmp/a /tmp/b /tmp/rom
exit $exit_code
