# Heatmap Program

## Overview
This project implements a program to generate a heatmap based on GPS track data. The program reads track points, processes them to form a heatmap, and then outputs the heatmap based on specific configurations provided through command-line arguments.

## Requirements
- C Compiler (e.g., GCC)
- Header files (`heatmap.h`, `track.h`, `trackpoint.h`, `location.h`) should be provided as part of the project
- Libraries:
  - Standard C libraries: `stdio.h`, `stdlib.h`, `string.h`, `stdbool.h`, `math.h`

## Compilation
To compile the program, run:

```bash
gcc -o heatmap heatmap.c track.c trackpoint.c location.c
./heatmap 10 10 "RGB" 5
<latitude> <longitude> <timestamp>
37.7749 -122.4194 1636500000
37.7750 -122.4195 1636500600
