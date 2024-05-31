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
LWIP_REPO_URL="https://github.com/ps2dev/lwip.git"
LWIP_REPO_FOLDER="common/external_deps/lwip"
LWIP_BRANCH_NAME="ps2-v2.0.3"
if test ! -d "$LWIP_REPO_FOLDER"; then
  git clone --depth 1 -b $LWIP_BRANCH_NAME $LWIP_REPO_URL "$LWIP_REPO_FOLDER"_inprogress || exit 1
  mv "$LWIP_REPO_FOLDER"_inprogress "$LWIP_REPO_FOLDER"
else
  (cd "$LWIP_REPO_FOLDER" && git fetch origin && git reset --hard "origin/${LWIP_BRANCH_NAME}" && git checkout "$LWIP_BRANCH_NAME" && cd - )|| exit 1
fi
