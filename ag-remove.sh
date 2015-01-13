#!/bin/sh

AG=ag

read -p "This will remove all project directories. Are you sure? " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]
then
    DIRS=`$AG proj dirs`
    for d in $DIRS 
    do
        rm -rf $d
    done
fi

