#!/bin/sh

self=$(basename "$0")

for file in *; do
  if test ! -f "$file" -o "$self" = "$file"; then
    continue
  fi
  cp "$file" "$HOME/.$file"
done
