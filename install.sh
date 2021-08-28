#!/bin/sh

set -e

if test -z "$HOME"; then
  echo "No \$HOME directory set." > /dev/stderr
  exit 1
fi

if test ! -d "$HOME"; then
  echo "'$HOME' is not a directory." > /dev/stderr
  exit 1
fi

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
