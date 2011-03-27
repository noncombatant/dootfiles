alias ls="'ls'"
alias la="ls -AF"
alias l="ls -F"
alias ll="ls -ltrh"
alias dis="objdump -Mintel -d"
alias igrep="grep -i"

function d {
      cd "$1"
      ls -F
}

umask 022

export PATH="${HOME}/bin:/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin:/usr/games:/usr/X11R6/bin:/usr/local/mysql-5.1.51-osx10.6-x86_64/bin:/usr/local/git/bin:$HOME/Downloads/android-sdk-mac_x86/tools:$HOME/Downloads/android-sdk-mac_x86/platform-tools"

export EDITOR=vim
export VISUAL=$EDITOR
export PAGER=less
export LC_ALL=en_US.UTF-8
export CFLAGS="-Wall -Werror -Wextra -O2 -pedantic -std=c99"

PS1="    \h:\w > "

