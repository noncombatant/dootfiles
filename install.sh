#!/bin/sh

here=$(dirname "$0")

for file in $here/doots/*; do
  f=$(basename "$file")
  cp "$file" "$HOME/.$f"
done
