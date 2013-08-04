alias ls="'ls'"
alias la="ls -AF"
alias l="ls -F"
alias ll="ls -ltrh"
alias dis="objdump -Mintel -d"
alias grep="grep -E"
alias igrep="grep -Ei"
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

umask 022

export PATH="${HOME}/bin:/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin:/usr/games:/usr/X11R6/bin:/usr/local/mysql-5.1.51-osx10.6-x86_64/bin:/usr/local/git/bin:$HOME/Downloads/android-sdk-mac_x86/tools:$HOME/Downloads/android-sdk-mac_x86/platform-tools"
export PATH="$HOME/bin:/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin:/usr/games:/usr/X11R6/bin:/Applications/Xcode.app/Contents/Developer/usr/bin:/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin"

export MANPATH="/usr/local/man:/usr/local/share/man:/usr/share/man:$MANPATH"

export EDITOR="/usr/bin/vim"
export VISUAL="$EDITOR"
export PAGER="/usr/bin/less"
export LC_ALL="en_US.UTF-8"
export CFLAGS="-Wall -Wextra -O2 -pedantic -std=c99 -I/Developer/SDKs/MacOSX10.6.sdk/usr/include"
export CPPFLAGS="-O2 -Wall -Wextra -pedantic"

PS1="    \H:\w \$ "

