#!/bin/bash

rc_files="gitconfig Xresources vimrc tmux.conf fvwm2rc bashrc"
home_bin="$HOME/bin"
here=$(dirname "$0")

mkdir -p "$home_bin"

for file in $rc_files
do
  cp "$here/$file" "$HOME/.$file"
done

for file in bin/*
do
  cp "$here/$file" "$home_bin/$(basename $i)"
done
