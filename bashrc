export PS1="\u@\h:\w \$ "
export EDITOR=vim
export VISUAL=$EDITOR
export BROWSER=google-chrome
export PATH="$HOME/depot_tools:$HOME/bin:$PATH"
export LC_ALL="en_US.UTF-8"

export CC=clang
export CFLAGS="-Wall -Wextra -Werror -O2 -std=c99"
export GOPATH="$HOME/gofun"

umask 022

alias ls="'ls'"
alias la="ls -AF"
alias l="ls -F"
alias ll="ls -ltrh"
alias dis="objdump -Mintel -d"

# Make searching use POSIX extended regular expressions and be case-insensitive.
alias igrep="grep -Ei"
alias less="less -i"

# Use POSIX regular expressions in tools.
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

function volume {
  declare -i v="$1"
  v=$(($v * 10))
  if test $v -lt 0; then v=0; fi
  if test $v -gt 100; then v=100; fi
  pactl set-sink-volume 0 $v%
  pactl set-sink-volume 1 $v%
}
