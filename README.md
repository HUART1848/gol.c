# Conway's Game of Life in C

## Features

* Generate grayscale .pgm images for subsequent animation (with `ffmpeg`) 
* In-memory image upscaling

## Examples

Pipe output directly to ffplay for direct visualisation:

```bash
./gol | ffplay -framerate 30 -i -
```
