#!/bin/bash

####################
# Lab 1 Exercise 5
# Name: Vikrant Prakash
# Student No: A0227630L
# Lab Group: 3
####################

# Fill the below up
hostname=$HOSTNAME
machine_hardware="$(uname) `echo "$(uname -m)"`"
max_user_process_count=$(cat /proc/sys/kernel/pid_max)
user_process_count=$(ps -u vikrant | wc -l)
user_with_most_processes=$(ps -eo user|sort|uniq -c|sort -n|tail -1|awk '{print $2}')
mem_free_percentage=$(free | grep Mem | awk '{print $4/$2 * 100.0}')

echo "Hostname: $hostname"
echo "Machine Hardware: $machine_hardware"
echo "Max User Processes: $max_user_process_count"
echo "User Processes: $user_process_count"
echo "User With Most Processes: $user_with_most_processes"
echo "Memory Free (%): $mem_free_percentage"

