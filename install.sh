#!/bin/sh

self=$(basename "$0")

for file in *; do
  if test ! -f "$file" -o "$self" = "$file"; then
    continue
  fi
  if ! cmp "$file" "$HOME/.$file"; then
    diff "$file" "$HOME/.$file"
    exit 1
  fi
  cp "$file" "$HOME/.$file"
done
