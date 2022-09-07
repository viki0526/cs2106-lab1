#!/bin/bash

####################
# Lab 1 Exercise 7
# Name:
# Student No:
# Lab Group:
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
    if [[ "${number}" =~ '^[0-9]+$' ]]; then
    echo INVALID
    echo "Please enter a valid number [0-9]:"
    else
    underscore="_"
    filename="$prefix$underscore$number"
    touch $filename
    ((i++))
    fi
    done




echo "Files Created"
ls *.txt
echo ""


# echo ""
# echo "Files Renamed"
# ls *.txt
# echo ""