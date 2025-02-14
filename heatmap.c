
#include "heatmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>


#include "track.h"
#include "trackpoint.h"
#include "location.h"

bool trackpoint_is_valid(double lat, double lon);

int main(int argc, char **argv)
{
    if(argc != 5){
        return  -1;
    }
  command_line args;
  if (!validate_command_line(argc, argv, &args))
    {
      fprintf(stderr, "%s: invalid parameter\n", argv[0]);
      return 1;
    }

  track *trk = track_create();
  if (trk == NULL)
    {
      fprintf(stderr, "%s: could not create track\n", argv[0]);
      return 1;
    }

  if (!read_input(stdin, trk))
    {
      track_destroy(trk);
      return 1;
    }

  // print the heatmap
  if (!show_heatmap(stdout, trk, &args))
    {
      track_destroy(trk);
      return 1;
    }

  track_destroy(trk);
  return 0;
}


// Validates command line arguments and stores them in the `args` struct.
// Returns true if valid, false otherwise.

bool validate_command_line(int argc, char **argv, command_line *args) {
    
    // Check if cell width and height are positive
    if (atof(argv[1]) <= 0 || atof(argv[2]) <= 0) {
        return false;
    }

    // Check if colors string is provided
    if (argv[3] == NULL) {
        return false;
    }

    // Check if range width is a positive integer
    if (strtol(argv[4], NULL, 10) <= 0) {
        return false;
    }

    // Assign values to the args struct
    args->cell_width = atof(argv[1]);
    args->cell_height = atof(argv[2]);
    args->colors = argv[3];
    args->num_colors = strlen(argv[3]);
    args->range_width = strtol(argv[4], NULL, 10);

    return true;
}

 
// Reads input from the file and populates the track with valid points.
// Returns true if the input is successfully processed, false otherwise.

bool read_input(FILE *in, track *trk) {
    double lat, lon;
    long time, last_time = -1;
    bool segment_started = false;

    while (true) {
        char line[300];
        if (!fgets(line, sizeof(line), in)) {  // End of file
            break;
        }

        if (line[0] == '\n') {  // Empty line, ends current segment
            if (segment_started) {
                segment_started = false;
                continue;
            } else {
                return false;
            }
        }

        if (sscanf(line, "%lf %lf %ld", &lat, &lon, &time) != 3) {  // Invalid line format
            return false;
        }

        if (!trackpoint_is_valid(lat, lon)) {  // Invalid coordinates
            fprintf(stderr, "Invalid latitude or longitude: %f, %f\n", lat, lon);
            return false;
        }

        if (time <= last_time) {  // Timestamps must increase
            fprintf(stderr, "Timestamps must be in increasing order: %ld\n", time);
            return false;
        }
        last_time = time;

        trackpoint *pt = trackpoint_create(lat, lon, time);  // Create a new point
        if (pt == NULL) {
            fprintf(stderr, "Could not create point %f %f %ld\n", lat, lon, time);
            return false;
        }

        track_add_point(trk, pt);  // Add point to track
        trackpoint_destroy(pt);  // Destroy temporary point object
        segment_started = true;
    }

    return segment_started;
}



// Generates a heatmap from the track data and writes it to the output file.
// Returns true on successful generation.

bool show_heatmap(FILE *out, const track *trk, const command_line *args) {
    size_t rows = 0, cols = 0;
    size_t **map_pointer;

    // Create heatmap based on track data and cell dimensions
    track_heatmap(trk, args->cell_width, args->cell_height, &map_pointer, &rows, &cols);

    // Loop through the heatmap grid
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            // Determine the correct color to display based on the map value
            if (map_pointer[i][j] >= (args->num_colors * args->range_width) - args->range_width) {
                fprintf(out, "%c", args->colors[args->num_colors - 1]);
            } else {
                fprintf(out, "%c", args->colors[(int)floor(map_pointer[i][j] / args->range_width)]);
            }
        }
        printf("\n");  // Newline after each row
    }

    // Free the allocated memory for the heatmap
    for (size_t i = 0; i < rows; i++) {
        free(map_pointer[i]);
    }
    free(map_pointer);
    
    return true;
}

// Validates latitude and longitude values.
bool trackpoint_is_valid(double lat, double lon) {
    return (lat >= -90.0 && lat <= 90.0) && (lon >= -180.0 && lon < 180.0);
}
