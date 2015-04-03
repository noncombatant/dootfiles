export PS1="\u@\h:\w \$ "
export EDITOR=vim
export VISUAL=$EDITOR
export BROWSER=google-chrome
export PATH="$HOME/depot_tools:$HOME/bin:$PATH"
export LC_ALL="en_US.UTF-8"

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

d() {
    x="$1"
    if [ x"$x" = x ]; then
        x=$(pwd)
    fi

    clear
    cd "$x"
    echo $(pwd)
    ls -F
}

run() {
    make "$1" && ./$@
}

get() {
    grep -Ei -A3 -B3 $@
}

mux() {
    tmux $@ || screen $@
}

csearch() {
  search -x /out/ -n '\.(h|c|cpp|cc|m|mm)$' $@
}

hgrep() {
  history | grep -iE "$@" | tail -n 10 | sed -E -e 's/^\s+[0-9]+\s+//'
}

try() {
  "$@" || (e=$?; echo "$@" > /dev/stderr; exit $e)
}
