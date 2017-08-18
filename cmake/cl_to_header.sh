#!/bin/sh

# Copyright (c) 2016-2017 Matthias Noack <ma.noack.pr@gmail.com>
#
# See accompanying file LICENSE and README for further information.

IN_FILE=$1
OUT_FILE=$2

VAR_NAME=$(basename $1 .sh)

# open raw string literal
echo 'R"str_not_in_src(' > $OUT_FILE
# copy source code
cat $IN_FILE >> $OUT_FILE
# close raw string literal
echo ')str_not_in_src"' >> $OUT_FILE

