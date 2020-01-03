#!/bin/sh
#
# Use to reverse a polygon
tail -r $1 > $$
mv -f $$ $1
