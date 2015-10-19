#!/bin/sh
# See http://www.wtfpl.net/txt/copying for license details
# Creates a minimal manifest and manifest.uuid file so sqlite (and fossil) can build
git rev-parse --git-dir >/dev/null || exit 1
git log -1 --format=format:%ci%n | sed -e 's/ [-+].*$//;s/ /T/;s/^/D /' > manifest
echo $(git log -1 --format=format:%H) > manifest.uuid
