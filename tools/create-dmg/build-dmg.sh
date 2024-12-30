#!/bin/bash

NAME=$1
# FIXME: Undo this overengineered crap once the following issue gets figured out:
#        https://github.com/actions/runner-images/issues/7522
max_tries=10
i=0
until create-dmg \
    --volname "$NAME" \
    --background "installer_background.png" \
    --window-pos 200 120 \
    --window-size 800 450 \
    --icon-size 100 \
    --icon "$NAME.app" 200 190 \
    --app-drop-link 600 185 \
    "$NAME.dmg" \
    "$NAME.app"
do
    if [ $i -eq $max_tries ]
    then
        echo 'Error: create-dmg did not succeed even after 10 tries.'
        exit 1
    fi
    i=$((i+1))
done
