# Conway's Game of Life in C

## Features

* Generate grayscale .pgm images for subsequent animation (with `ffmpeg`) 
* In-memory image upscaling

## Examples

Pipe output directly to ffplay for direct visualisation:

```bash
./gol -w 100 -h 100 -s 4 -n 300 | ffplay -framerate 30 -i -
```
