#!/bin/bash
# download_depdencies.sh by Francisco Javier Trujillo Mata (fjtrujy@gmail.com)

## Download LWIP
REPO_URL="https://github.com/ps2dev/lwip.git"
REPO_FOLDER="common/external_deps/lwip"
BRANCH_NAME="ps2-v2.0.3"
if test ! -d "$REPO_FOLDER"; then
  git clone --depth 1 -b $BRANCH_NAME $REPO_URL "$REPO_FOLDER" || exit 1
else
  (cd "$REPO_FOLDER" && git fetch origin && git reset --hard "origin/${BRANCH_NAME}" && git checkout "$BRANCH_NAME" && cd - )|| exit 1
fi
