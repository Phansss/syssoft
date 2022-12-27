#!/bin/sh

rm $0

"./mainproblem"

echo "

------------------
(program exited with code: $?)" 		


echo "Press return to continue"
#to be more compatible with shells like bash
dummy_var=""
read dummy_var
