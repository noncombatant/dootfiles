export EDITOR=vim
export VISUAL=$EDITOR
export LC_ALL="en_US.UTF-8"
export PATH="$HOME/bin:$PATH"
export PATH="$HOME/.cargo/bin:$PATH"
export MANWIDTH=80

export CC=clang
export CFLAGS="-Weverything -Werror -O0 -std=c11"
export CXX=clang++
export CXXFLAGS="-Weverything -Werror -O0 -std=c++17"

umask 022

alias py=python3

alias ls="'ls'"
alias l="ls -AF"
alias ll="ls -ltrhA"

alias igrep="grep -Ei"
alias less="less -i"

alias grep="grep -E"
if echo | sed -r > /dev/null 2>&1; then
  alias sed="sed -r"
elif echo | sed -E > /dev/null 2>&1; then
  alias sed="sed -E"
fi

if which otool > /dev/null; then
  alias dis="otool -tV"
elif which objdump > /dev/null; then
  alias dis="objdump -Mintel -d"
fi

alias mv="mv -i"
alias cp="cp -i"
alias ln="ln -i"

function d {
  declare dir
  if test $# -eq 0; then
    dir=$(pwd)
  elif test -d "$1"; then
    dir="$1"
  else
    # find -E . -type d -not -path '*/.*' -iregex '.*'"$1"'.*'
    dir=$(search -t d -n "$1" . | head -n1)
  fi

  cd "$dir" && echo "[1m$dir[0m" && ls -F
}

function = {
  echo "$*" | bc -l
}

function get {
  grep -Ei -A3 -B3 "$@"
}

function mux {
  tmux "$@" || screen "$@"
}

function searchc {
  search -x /out/ -n '\.(h|c|cpp|cc|m|mm)$' "$@"
}

function hgrep {
  history | grep -iE "$@" | tail -n 10
}

function udate {
  date -u +'%d %b %Y %H:%M UTC'
}

function goatoc {
  godoc "$1" | less -p "${2-package $1}";
}

function remove-dots { 
  for i in *.mp3; do
    j=$(echo "$i" | sed -e 's/([0-9][0-9])\./\1/');
    mv -i "$i" "$j";
  done
}

function prepend-disc { 
  for i in *.mp3; do
    mv -i "$i" $1-"$i";
  done
}

function prepend-artist {
  for i in *.mp3; do
    j=$(echo "$i" | sed -e "s/([-0-9]+)(.+)/\1 $1 -\2/")
    mv "$i" "$j"
  done
}

function gofmtall {
  gofmt -s -w *.go
}
