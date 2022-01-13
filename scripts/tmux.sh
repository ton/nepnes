#!/bin/sh
set -ex

session="nepnes"

tmux new-session -d -s "$session"

window="1"
tmux split-window -d -t "$session:$window" -v -l 8 'scripts/tail-debug-pipe.sh'
tmux send-keys -t "$session:$window" 'source init-sandbox.sh' C-m C-l

window="2"
tmux new-window -d -t "$session"
tmux send-keys -t "$session:$window" 'source init-sandbox.sh; cd src' C-m C-l

exec tmux attach-session -t "$session"
