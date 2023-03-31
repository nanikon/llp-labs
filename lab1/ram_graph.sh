#!/bin/bash

cur_pid=122823

rm -rf ram_data

for i in {1..7200}
do
    buf=$(ps -o vsz= $cur_pid)
    echo $i","$buf >> ram_data
    sleep 1
done
