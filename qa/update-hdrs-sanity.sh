#!/usr/bin/env bash

# TODO: fix that mess
echo TODO: fix it first 1>&2
exit 1

# FIXME: check git too
for i in `find ../include/clapi/transforms -maxdepth 1 -iname '*.hh' -type f`
do
# FIXME: missing mkdir -p
# FIXME: realpath repeated.
echo "#include \"clapi/`realpath -m --relative-to=../include/clapi ${i}`\"" > 'headers-work/'$(dirname `realpath -m --relative-to=../include/ ${i}`)/`basename ${i%.*}.cc`
done

