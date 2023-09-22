#!/bin/bash

NAME=$1
create-dmg --volname "$NAME" --background "installer_background.png" --window-pos 200 120 --window-size 800 450 --icon-size 100 --icon "$NAME.app" 200 190 --app-drop-link 600 185 "$NAME.dmg" "$NAME.app"
