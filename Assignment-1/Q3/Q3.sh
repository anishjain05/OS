#!/bin/bash
directory_name="Result"
mkdir -p "$directory_name"
while read -r x y op; 
do
    result=0
    if [[ $op == "xor" ]]; then
        result=$((x ^ y))
    elif [[ $op == "product" ]]; then
        result=$((x * y))
    elif [[ $op == "compare" ]]; then
        if ((x > y)); then
            result=$x
        else
            result=$y
        fi
    else    
        echo "Unknown operation: $op"
        continue
    fi
    echo "$result" >> "$directory_name/output.txt"
done < input.txt