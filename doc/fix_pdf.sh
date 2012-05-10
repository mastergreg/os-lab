#!/bin/bash
QUALITY=100
GEOMETRY=520x805+167+160
DENSITY=600x600
INPUT=$(pwd)/$1

if [ x$1 == 'x' ]
then
    echo No input
    exit -1
fi
if [ x$2 == 'x' ]
then
    OUTPUT=$(pwd)/${1/\.pdf/_fixed.pdf}
else
    OUTPUT=$(pwd)/$2
fi

let i=1
while [[ -a $OUTPUT ]];
do
    OUTPUT=${OUTPUT/_fixed*.pdf/_fixed_$i.pdf}
    let i++
done
convert -crop $GEOMETRY -quality $QUALITY -density $DENSITY "$INPUT" "page-%03d.png"
convert *.png +adjoin "$OUTPUT"
