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
  echo "$@" > /dev/stderr
  exit 1
}

usage() {
  echo "${help-BUG: This program has no help text.}"
  exit 1
}

cleanup() {
  exit "$1"
}

source "$HOME"/bin/lib.sh

# Show $help if necessary.
if [[ $# > 0 ]]; then
  __1=$(echo "$1" | tr "[:upper:]" "[:lower:]")
  [[ "$__1" = help || "$__1" = "--help" || "$__1" = "-h" ]] && usage || true
fi
