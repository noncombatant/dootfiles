export PS1="\u@\h:\w $ "
export EDITOR=vim
export VISUAL=$EDITOR
export BROWSER=google-chrome
export PATH="$HOME/bin:$PATH"
export LC_ALL="en_US.UTF-8"

alias ls="'ls'"
alias la="ls -AF"
alias l="ls -F"
alias ll="ls -ltrh"
alias dis="objdump -Mintel -d"
alias grep="grep -E"
alias igrep="grep -Ei"
alias sed="sed -r"
alias mv="mv -i"
alias cp="cp -i"

function d {
    x="$1"
    if [ x"$x" = x ]; then
        x=$(pwd)
    fi

    clear
    cd "$x"
    echo $(pwd)
    ls -F
}

function run {
    make "$1"
    ./$@
}
