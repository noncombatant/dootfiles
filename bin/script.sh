# https://github.com/tests-always-included/wick/blob/master/doc/bash-strict-mode.md
set -o errexit
set -o errtrace
set -o nounset
set -o pipefail
shopt -s extdebug
IFS=$'\n\t'
trap 'cleanup $?' ERR

source "$HOME"/bin/lib.sh
