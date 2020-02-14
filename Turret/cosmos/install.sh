#!/bin/bash
awk '/^    \$ /{sub(/^[^\$]+\$ /, "", $0); print $0}
       /^    # / {sub(/^[^#]*\# /,  "", $0); print "sudo " $0}' README.md | bash
