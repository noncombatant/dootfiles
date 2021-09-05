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

# Install the modern goodies:

mkdir foo

cd foo

# bat
git clone https://github.com/sharkdp/bat.git
cd bat
cargo install --locked bat
which bat
cd ..

# fd
git clone https://github.com/sharkdp/fd.git
cd fd
cargo install fd-find
which fd
cd ..

# ripgrep
git clone https://github.com/BurntSushi/ripgrep.git
cd ripgrep
cargo install ripgrep
which rg
cd ..

# fzf
git clone https://github.com/junegunn/fzf.git
cd fzf
make install
cp bin/fzf ~/bin/fzf
which fzf
cd ..

# search
git clone https://github.com/noncombatant/search.git
cd search
go build
mv search ~/bin
which search

cd ..
rm -rf foo
