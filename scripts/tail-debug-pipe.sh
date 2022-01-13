#!/bin/sh
pipe_name="debug-pipe"
if [ ! -p "$pipe_name" ]
then
    mkfifo "$pipe_name"
fi

less +F -f "$pipe_name"
