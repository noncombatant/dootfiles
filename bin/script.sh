#!/usr/bin/env bash

# https://github.com/tests-always-included/wick/blob/master/doc/bash-strict-mode.md
set -o errexit
set -o errtrace
set -o nounset
set -o pipefail
shopt -s extdebug
IFS=$'\n\t'
trap 'cleanup $?' ERR

error() {
  echo "$1" > /dev/stderr
  exit 1
}

usage() {
  echo "$help"
  exit 1
}

cleanup() {
  exit $?
}

source "$HOME"/bin/lib.sh
