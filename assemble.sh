#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
echo "#include <string>"
echo "std::string grammarText = (R\"("
cat "$DIR"/CafeBabe.peg | sed 's/"/\\"/g'
echo ")\");"