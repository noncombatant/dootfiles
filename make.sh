#!/bin/sh

rcFiles="bashrc gitconfig Xresources indent.pro muttrc vim vimrc vimrc-mutt"
home="$HOME"
homeBin="$home/bin"

function install() {
    mkdir -p "$homeBin"

    for i in $rcFiles
    do
        cp -i -a "$i" "$home"/."$i"
    done

    for i in bin/*
    do
        cp -i -a "$i" "$homeBin"/"$(basename $i)"
    done
}

function slurp() {
    for i in $rcFiles
    do
        cp -f -a "$home"/."$i" "$i"
    done

    for i in bin/*
    do
        cp -f -a "$homeBin"/"$(basename $i)" "$i"
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

