alias ls="'ls'"
alias la="ls -AF"
alias l="ls -F"
alias ll="ls -ltrh"
alias dis="objdump -Mintel -d"
alias grep="grep -E"
alias igrep="grep -i"
alias mv="mv -i"
alias cp="cp -i"

function d {
      cd "$1"
      ls -F
}

umask 022

export PATH="${HOME}/bin:/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin:/usr/games:/usr/X11R6/bin:/usr/local/mysql-5.1.51-osx10.6-x86_64/bin:/usr/local/git/bin:$HOME/Downloads/android-sdk-mac_x86/tools:$HOME/Downloads/android-sdk-mac_x86/platform-tools"

export EDITOR="/usr/bin/vim"
export VISUAL="$EDITOR"
export PAGER="/usr/bin/less"
export LC_ALL="en_US.UTF-8"
export CFLAGS="-Wall -Wextra -O2 -pedantic -std=c99"

PS1="    \H:\w \$ "

