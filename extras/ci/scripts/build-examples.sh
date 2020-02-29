#!/bin/bash

# This script verifies that every example in the examples directory can be built

set -e

TMPDIR="$(pwd)/.tmp"
CIDIR="$(pwd)/extras/ci"
EXAMPLEDIR="$(pwd)/examples"

# Re-create build directory
if [ -d "$TMPDIR" ]; then
  rm -r "$TMPDIR"
fi
mkdir -p "$TMPDIR"

# For each example
for EXAMPLE in "$EXAMPLEDIR"/*; do
  # Check that an .ino file exists
  EXAMPLENAME=$(basename $EXAMPLE)
  if [ -d "$EXAMPLE" ] && [ -f "$EXAMPLE/$EXAMPLENAME.ino" ]; then
    MAINCPP="$TMPDIR/$EXAMPLENAME/src/main.cpp"
    INOFILE="$TMPDIR/$EXAMPLENAME/src/$EXAMPLENAME.ino"
    PROJECTDIR="$TMPDIR/$EXAMPLENAME"
    echo "Building $EXAMPLENAME"
    echo "------------------------------------------------------------"
    mkdir -p "$PROJECTDIR/lib"
    cp -r "$EXAMPLEDIR/$EXAMPLENAME/." "$PROJECTDIR/src"
    ln -s "$(pwd)" "$PROJECTDIR/lib/pio-esp32-ci-demo"
    cp "$CIDIR/templates/example-platformio.ini" "$PROJECTDIR/platformio.ini"
    echo "#include <Arduino.h>" > "$MAINCPP"
    cat "$INOFILE" >> "$MAINCPP"
    rm "$INOFILE"
    pio run -d "$TMPDIR/$EXAMPLENAME"
  fi
done
