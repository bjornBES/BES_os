#!/bin/bash
# link.sh: Wrapper script for invoking the linker

# First argument is the linker
LINKER="$1"
shift

# The rest are arguments to pass to the linker
"$LINKER" "$@" > linkererrors.txt 2>&1