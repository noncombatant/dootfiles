#!/bin/sh

set -e

here=$(dirname "$0")

for file in $here/doots/*; do
  f=$(basename "$file")
  cp "$file" "$HOME/.$f"
done

bin="$HOME/bin"
mkdir -p "$bin"

for file in $here/bin/*; do
  f=$(basename "$file")
  cp "$file" "$bin"
done
