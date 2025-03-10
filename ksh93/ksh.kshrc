# If you want to edit or add to this file,
# create $HOME/.kshrc and put it in there. Only this file and
# $HOME/.kshrc are sourced when interactive.

PS1=$'\E[1;91m$(/usr/bin/logname)@$(/usr/bin/hostname) \E[1;33m${PWD/~(Sl-r)$HOME/"~"}\E[0m$ '

# You can change to vi if you want to.
set -o emacs
