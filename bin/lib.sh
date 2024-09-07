#!/usr/bin/env bash

udate() {
  date -u +'%Y-%m-%d-%H:%M:%S'
}

realpath() {
  echo "$(cd "$(dirname "$1")" && pwd -P)/$(basename "$1")"
}
