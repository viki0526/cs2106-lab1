#!/bin/bash

####################
# Lab 1 Exercise 7
# Name: Vikrant Prakash
# Student No: A0227630L
# Lab Group: 3
####################

####################
# Strings that you may need for prompt (not in order)
####################
# Enter $N numbers:
# Enter NEW prefix (only alphabets):
# Enter prefix (only alphabets):
# INVALID
# Number of files to create:
# Please enter a valid number [0-9]:
# Please enter a valid prefix [a-z A-Z]:

####################
# Strings that you may need for prompt (not in order)
####################

#################### Steps #################### 

# Fill in the code to request user for the prefix

# Check the validity of the prefix #

# Enter numbers and create files #

# Fill in the code to request user for the new prefix

# Renaming to new prefix #


echo "Enter prefix (only alphabets):"

j=0
while [ $j -lt 5 ]
do
read prefix
if [[ "${prefix}" =~ [^a-zA-Z] ]]; then
   echo INVALID
   echo "Please enter a valid prefix [a-z A-Z]:"
else
    break
fi
done

echo "Number of files to create:"
read file_count

echo "Enter $file_count numbers"
i=1
while [ $i -le $file_count ]
    do
    read number
    re='^[0-9]+$'
    if ! [[ $number =~ $re ]]; then
    echo INVALID
    echo "Please enter a valid number [0-9]:"
    else
    underscore="_"
    ext=".txt"
    filename="$prefix$underscore$number$ext"
    touch $filename
    ((i++))
    fi
    done



echo ""
echo "Files Created"
ls *.txt

echo ""

echo "Enter NEW prefix (only alphabets):"

k=0
while [ $k -lt 5 ]
    do
    read new_prefix
    if [[ "${new_prefix}" =~ [^a-zA-Z] ]]; then
        echo INVALID
        echo "Please enter a valid prefix [a-z A-Z]:"
    else
        break
    fi
    done

for file in *.txt
    do
        [ -f "$file" ] || continue
        mv "$file" "$new_prefix${file//$prefix/}"
    done

echo ""
echo "Files Renamed"
ls *.txt