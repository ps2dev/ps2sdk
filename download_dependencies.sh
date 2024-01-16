#!/bin/bash
# download_depdencies.sh by Francisco Javier Trujillo Mata (fjtrujy@gmail.com)

## Protect reentrancy of this script if possible
if [ "x$1" != "xlocked" ]; then
  if command -v flock > /dev/null; then
    flock "$0" "$BASH" "$0" locked
    exit $?
  fi
  if command -v perl > /dev/null; then
    perl -MFcntl=:flock -e '$|=1; $f=shift; open(FH,$f) || die($!); flock(FH,LOCK_EX); my $exit_value = system(@ARGV); flock(FH,LOCK_UN); exit($exit_value);' "$0" "$BASH" "$0" locked
    exit $?
  fi
fi

## Download LWIP
REPO_URL="https://github.com/ps2dev/lwip.git"
REPO_FOLDER="common/external_deps/lwip"
BRANCH_NAME="ps2-v2.0.3"
if test ! -d "$REPO_FOLDER"; then
  git clone --depth 1 -b $BRANCH_NAME $REPO_URL "$REPO_FOLDER"_inprogress || exit 1
  mv "$REPO_FOLDER"_inprogress "$REPO_FOLDER"
else
  (cd "$REPO_FOLDER" && git fetch origin && git reset --hard "origin/${BRANCH_NAME}" && git checkout "$BRANCH_NAME" && cd - )|| exit 1
fi
