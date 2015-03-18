#!/bin/sh

rc_files="bashrc gitconfig Xresources muttrc vimrc"
home_bin="$HOME/bin"

function install() {
    mkdir -p "$home_bin"

    for i in $rc_files
    do
        cp -i -a "$i" "$HOME"/."$i"
    done

    for i in bin/*
    do
        cp -i -a "$i" "$home_bin"/"$(basename $i)"
    done
}

function slurp() {
    for i in $rc_files
    do
        cp -f -a "$HOME"/."$i" "$i"
    done

    for i in bin/*
    do
        cp -f -a "$home_bin"/"$(basename $i)" "$i"
    done

    git status
}

case "$1" in 
    install)
        install
        ;;
    slurp)
        slurp
        ;;
    *)
        echo "Usage: $0 {install|slurp}"
        exit 1
esac
