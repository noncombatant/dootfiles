export PS1="\h:\w \$ "
export EDITOR=vim
export VISUAL=$EDITOR
export BROWSER=google-chrome
export LC_ALL="en_US.UTF-8"
export PATH="$HOME/bin:$PATH"

export CC=clang
export CFLAGS="-Wall -Wextra -Werror -O0 -ansi -pedantic -std=c11"
export CXX=clang++
export CXXFLAGS="-Wall -Wextra -Werror -O0 -ansi -pedantic -std=c++11"
export GOPATH="$HOME/gofun"

umask 022

alias ls="'ls'"
alias la="ls -AF"
alias l="ls -F"
alias ll="ls -ltrh"
if which objdump > /dev/null; then
  alias dis="objdump -Mintel -d"
elif which otool > /dev/null; then
  alias dis="otool -tV"
fi

alias igrep="grep -Ei"
alias less="less -i"

alias grep="grep -E"
if echo | sed -r > /dev/null 2>&1; then
  alias sed="sed -r"
elif echo | sed -E > /dev/null 2>&1; then
  alias sed="sed -E"
fi

# Don't overwrite files.
alias mv="mv -i"
alias cp="cp -i"
alias ln="ln -i"

function d {
  declare x="$1"
  if test -z "$x"
  then
    x=$(pwd)
  fi

  clear
  cd "$x"
  echo $(pwd)
  ls -F
}

function = {
  echo "$*" | bc -l
}

function run {
  make "$1" && ./$@
}

function get {
  grep -Ei -A3 -B3 $@
}

function mux {
  tmux $@ || screen $@
}

function csearch {
  search -x /out/ -n '\.(h|c|cpp|cc|m|mm)$' $@
}

function hgrep {
  history | grep -iE "$@" | tail -n 10
}

function try {
  "$@" || (e=$?; echo "$@" > /dev/stderr; exit $e)
}

function udate {
  date -u +'%d %b %Y %H:%M UTC'
}

function goatoc { godoc "$1" | less -p "${2-package $1}"; }

PATH="/Library/Frameworks/Python.framework/Versions/3.5/bin:${PATH}"
export PATH
