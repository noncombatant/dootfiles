#!/bin/sh

rc_files="bashrc gitconfig Xresources vimrc"
home_bin="$HOME/bin"

mkdir -p "$home_bin"

for i in $rc_files
do
    cp -i -a "$i" "$HOME"/."$i"
done

for i in bin/*
do
    cp -i -a "$i" "$home_bin"/"$(basename $i)"
done