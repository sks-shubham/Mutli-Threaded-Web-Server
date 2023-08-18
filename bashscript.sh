#!/bin/bash
rm -r $2

if [ ! -d $2 ] ; then
        mkdir $2 ;    
    fi

for ((i=1; i<=$3; i++))
do
    curl "http://127.0.0.1:$1/" > "$2/response$i.html"
done
