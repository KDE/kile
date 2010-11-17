#!/bin/sh
version="beta 5"

convert kile_splash.png -fill red -style Normal -pointsize 30 -gravity "Center" -draw "text 40,-24 '$version'"  "kile_splash_$version.png"
