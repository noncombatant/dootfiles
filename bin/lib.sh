#!/usr/bin/env bash

udate() {
  date -u +'%Y-%m-%d-%H:%M:%S'
}

core_count() {
  sysctl -n hw.physicalcpu 2> /dev/null || \
  grep -c ^processor /proc/cpuinfo 2> /dev/null || \
  echo 1
}

realpath() {
  echo "$(cd "$(dirname "$1")" && pwd -P)/$(basename "$1")"
}
