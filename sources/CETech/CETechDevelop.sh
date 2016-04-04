#!/bin/sh

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

LD_LIBRARY_PATH=$SCRIPTPATH
export LD_LIBRARY_PATH

mono $SCRIPTPATH/CETechDevelop.exe $@
