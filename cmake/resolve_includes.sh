#!/bin/bash

# Copyright (c) 2016-2017 Matthias Noack <ma.noack.pr@gmail.com>
#
# See accompanying file LICENSE and README for further information.

# Resolve include directives, intended for OpenCL code generation
# NOTE:
#	- handles exactly one layer of includes, because that's all what we need right now
#		- could be run in a loop until no includes are found in the file anymore

AWK_BIN="$1"
IN_FILE="$2"
INCLUDE_DIR="$3" # directory to which the include directives are relative to
OUT_FILE="$4"

TMP_FILE_A="${OUT_FILE}.a.tmp"
TMP_FILE_B="${OUT_FILE}.b.tmp"

# pattern to match a simple include directive:
# 	starts with optional space, 
#	followed by '#', 
#	followed by optional space, 
#	followed by 'include',
#	followed by at least some space,
#	followed by opening '"'
#	followed by arbitrary string, which is the first subgroup
#	followed by closing '"'
#	followed by arbitrary string, e.g. an inline comment
export PATTERN='^\s*#\s*include\s+"(.*)".*'


# awk command to replace include by included file content
#	if current line ($0) matches the pattern
#		extract the filename using gensub to replace the current line with the first (\\1) regex subgroup
#		read the whole file, printing it line by line
#		close the file
#	else, i.e. a non-include line
#		just print the line ($0)
function resolve_includes() {
	local FILE_IN=$1
	local FILE_OUT=$2
	INCLUDE_DIR=$INCLUDE_DIR $AWK_BIN '{if(match($0, ENVIRON["PATTERN"])) { file = gensub(ENVIRON["PATTERN"], "\\1", 1, $0); while ((getline l < (ENVIRON["INCLUDE_DIR"] "/" file)) > 0) { print l } close(file) } else { print $0 } }' "${FILE_IN}" > "${FILE_OUT}"
}

# copy IN_FILE
cp ${IN_FILE} ${TMP_FILE_A}

FILE_A="${TMP_FILE_A}"
FILE_B="${TMP_FILE_B}"
resolve_includes ${FILE_A} ${FILE_B}

# while there was change, try again until no includes remain
while ! diff "${TMP_FILE_A}" "${TMP_FILE_B}" > /dev/null; do 
	# swap file names
	TMP_VAR="${FILE_A}"
	FILE_A="${FILE_B}"
	FILE_B="${TMP_VAR}"
	
	resolve_includes ${FILE_A} ${FILE_B}
done

cp "${FILE_B}" "${OUT_FILE}"

rm -f "${TMP_FILE_A}" "${TMP_FILE_B}"

