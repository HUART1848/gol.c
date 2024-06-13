#!/usr/bin/env bash

ffmpeg -y -r 10 -i out/%d.pgm -c:v libx264 -r 30 out.mkv
