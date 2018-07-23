#!/bin/bash
cat duplicates.csv | while read line 
do
		  x=$( printf "%x" $line ) ; echo $x
done
