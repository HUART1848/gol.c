# Conway's Game of Life in C

## Features

* Generate grayscale .pgm images for subsequent animation (with `ffmpeg`) 
* In-memory image upscaling

## Examples

Pipe output directly to ffmpeg and ffplay for direct visualisation:

```bash
./gol | ffmpeg -y -r 15 -f pgm_pipe -i - -c:v libx264 -r 30 -f matroska - | ffplay -i -
```
