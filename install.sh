#!/bin/sh

rc_files="profile gitconfig Xresources vimrc tmux.conf"
home_bin="$HOME/bin"

mkdir -p "$home_bin"

for i in $rc_files
do
    cp "$i" "$HOME/.$i"
done

for i in .bashrc .shrc
do
    ln -f "$HOME/.profile" "$HOME/$i"
done

for i in bin/*
do
    cp "$i" "$home_bin/$(basename $i)"
done
