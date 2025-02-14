/**
 * Starter code for the track ADT.
 * You're going to get crashes out of just about every test until you
 * fix track_add_point and track_get_point to at least respect the ownership
 * of the points passed back and forth, so fix that first!
 *
 * @author CPSC 223 Staff and you
 */

#include "track.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>

trackpoint **get_sorted_trackpoints_by_longitude(track *trk, size_t *total_pts);
int compare_by_longitude(const void *a, const void *b);
void get_grid_indices(double lat, double lon, double big_lat, double small_lat, 
                      double start_lon, double end_lon, double cell_width, double cell_height, 
                      size_t num_rows, size_t num_cols, size_t *row_idx, size_t *col_idx);

typedef struct
{
  size_t num_pts;
  trackpoint **pts;
  double distance;
  size_t size;
  // TO DO: make dynamically allocated to allow > 10 pts
  // TO DO: add whatever else you need here!
} segment;

// TO DO: implement some more segment functions; keep things modular!
/**
 * Adds the given point to the given segment.
 *
 * @param seg a pointer to a segment, non-NULL
 * @param pt a pointer to a track point, non-NULL
 */
void segment_add_point(segment *seg, const trackpoint *pt);

struct track {
  segment **segs;
  size_t num_seg;
  size_t size;
};

// Creates a new segment, initializes its fields, and returns a pointer to it.
segment *seg_create() {
    segment *new_seg = malloc(sizeof(segment));  // Allocate memory for the segment
    new_seg->num_pts = 0;        // Initialize the number of points
    new_seg->distance = 0;       // Initialize the distance
    new_seg->size = 20;          // Set initial size for the points array
    new_seg->pts = malloc(sizeof(trackpoint*) * new_seg->size);  // Allocate memory for points array
    return new_seg;
}


// Deletes a segment, freeing all its trackpoints and the segment itself.
void seg_delete(segment *seg) {
    if (seg == NULL) {
        return; 
    }

    // Free all trackpoints in the segment
    for (int i = 0; i < seg->num_pts; i++) {
        if (seg->pts[i] != NULL) {
            trackpoint_destroy(seg->pts[i]);
        }
    }

    // Free the segment's points array and the segment itself
    free(seg->pts);
    free(seg);
}

// Creates a track with an initial segment and returns a pointer to it.
track *track_create() {
    track *trk = malloc(sizeof(*trk));
    trk->num_seg = 0;
    trk->size = 20;  // Initial size for the track's segment array
    trk->segs = malloc(sizeof(segment*) * trk->size);

    // Create the first segment and add it to the track
    trk->segs[0] = seg_create();
    trk->num_seg++;
    return trk;
}

// Destroys the track, freeing all its segments and the track itself.
void track_destroy(track *trk) {
    if (trk == NULL) {
        return;
    }

    // Free all segments in the track
    for (int i = 0; i < trk->num_seg; i++) {
        seg_delete(trk->segs[i]);
    }

    // Free the segment array and the track itself
    free(trk->segs);
    free(trk);
}

// Returns the number of segments in the track.
size_t track_count_segments(const track *trk) {
    return trk->num_seg;
}

// Returns the number of points in a specific segment of the track.
// Returns -1 if the segment index is invalid.
size_t track_count_points(const track *trk, size_t i) {
    if (trk->num_seg <= i) {
        return -1;
    }
    return trk->segs[i]->num_pts;
}

// Returns a copy of the trackpoint at a specific index in a segment.
// Returns NULL if the segment or point index is invalid.
trackpoint *track_get_point(const track *trk, size_t i, size_t j) {
    if (trk->num_seg <= i || trk->segs[i]->num_pts <= j) {
        return NULL;
    }
    return trackpoint_copy(trk->segs[i]->pts[j]);
}

// Calculates and returns an array of segment lengths based on the points in the track.
// Each length represents the total distance between points in that segment.
double *track_get_lengths(const track *trk) {
    location curr_loc, next_loc;
    double dist = 0;

    // Allocate memory for storing the length of each segment
    double *ans = malloc(sizeof(double) * trk->num_seg);

    // Loop through each segment
    for (size_t i = 0; i < trk->num_seg; i++) {
        dist = 0;  // Reset distance for the current segment

        // Calculate the distance between consecutive points
        for (size_t j = 0; j < trk->segs[i]->num_pts - 1; j++) {
            curr_loc = trackpoint_location(trk->segs[i]->pts[j]);
            next_loc = trackpoint_location(trk->segs[i]->pts[j + 1]);
            dist += location_distance(&curr_loc, &next_loc);
        }
        ans[i] = dist;  // Store the segment's total distance
    }

    return ans;
}


// Adds a trackpoint to the last segment of the track.
void track_add_point(track *trk, const trackpoint *pt) {
    if (trk->num_seg == 0) {
        return;  // No segments to add the point to
    }
    
    // Create a copy of the trackpoint and add it to the last segment
    trackpoint *new_trk = trackpoint_copy(pt);
    if (new_trk == NULL) {
        return;
    }

    segment_add_point(trk->segs[trk->num_seg - 1], new_trk);
    trackpoint_destroy(new_trk);  // Destroy the copied trackpoint after adding
}

// Starts a new segment in the track.
void track_start_segment(track *trk) {
    // If the segment array is full, expand its size
    if (trk->num_seg == trk->size) {
        segment **new_seg_arr = realloc(trk->segs, sizeof(segment*) * (trk->size * 2));
        trk->segs = new_seg_arr;
        trk->size *= 2;
    }

    // Create and add a new segment
    trk->segs[trk->num_seg] = seg_create();
    trk->num_seg++;
}

// Merges segments from start to end indices, keeping points in the start segment.
void track_merge_segments(track *trk, size_t start, size_t end) {
    // Check for valid range of segments
    if (start >= end || end >= trk->num_seg) {
        return;
    }

    // Move points from segments in the range into the start segment
    for (size_t i = start + 1; i < end; i++) {
        for (size_t j = 0; j < trk->segs[i]->num_pts; j++) {
            segment_add_point(trk->segs[start], trk->segs[i]->pts[j]);
        }
        seg_delete(trk->segs[i]);  // Delete the segment after merging
    }

    // Shift remaining segments to close the gap
    size_t shift = end - start - 1;
    for (size_t i = end; i < trk->num_seg; i++) {
        trk->segs[i - shift] = trk->segs[i];
    }

    trk->num_seg -= shift;  // Adjust the segment count
}



// Generates a heatmap based on the track's trackpoints, calculating the necessary rows and columns.
void track_heatmap(const track *trk, double cell_width, double cell_height,
                   size_t ***map, size_t *rows, size_t *cols) {
    size_t num_pts;
    trackpoint **track_pt_lst = get_sorted_trackpoints_by_longitude((track *)trk, &num_pts);

    // Initialize latitude bounds
    double big_lat = trackpoint_location(track_pt_lst[0]).lat;
    double small_lat = trackpoint_location(track_pt_lst[0]).lat;

    // Determine latitude bounds from trackpoints
    for (size_t i = 1; i < num_pts; i++) {
        double lat = trackpoint_location(track_pt_lst[i]).lat;
        if (lat > big_lat) {
            big_lat = lat;
        } else if (lat < small_lat) {
            small_lat = lat;
        }
    }

    // Initialize longitude bounds
    double start_lon = trackpoint_location(track_pt_lst[0]).lon;
    double end_lon = trackpoint_location(track_pt_lst[num_pts - 1]).lon;
    double max_dist_lon = (180 + start_lon) + (180 - end_lon);

    // Check for gaps in longitude between points
    for (size_t i = 0; i < num_pts - 1; i++) {
        double gap = trackpoint_location(track_pt_lst[i + 1]).lon - trackpoint_location(track_pt_lst[i]).lon;
        if (max_dist_lon < gap) {
            max_dist_lon = gap;
            start_lon = trackpoint_location(track_pt_lst[i + 1]).lon;
            end_lon = trackpoint_location(track_pt_lst[i]).lon;
        }
    }

    // Handle wraparound gap at the longitude boundaries
    double wraparound_gap = (180 - trackpoint_location(track_pt_lst[num_pts - 1]).lon) + (180 + trackpoint_location(track_pt_lst[0]).lon);
    if (max_dist_lon < wraparound_gap) {
        max_dist_lon = wraparound_gap;
        start_lon = trackpoint_location(track_pt_lst[0]).lon;
        end_lon = trackpoint_location(track_pt_lst[num_pts - 1]).lon;
    }

    // Calculate the number of rows and columns for the heatmap
    *rows = ceil((big_lat - small_lat) / cell_height);
    double direct_diff = fabs(start_lon - end_lon);
    double wraparound_diff = 360.0 - direct_diff;
    *cols = ceil(fmin(direct_diff, wraparound_diff) / cell_width);

    // Allocate memory for the heatmap
    *map = malloc(sizeof(size_t *) * (*rows));
    for (size_t i = 0; i < (*rows); i++) {
        (*map)[i] = calloc(*cols, sizeof(size_t));
    }

    // Populate the heatmap
    size_t row_index = 0;
    size_t col_index = 0;
    for (size_t i = 0; i < num_pts; i++) {
        get_grid_indices(trackpoint_location(track_pt_lst[i]).lat, trackpoint_location(track_pt_lst[i]).lon,
                         big_lat, small_lat, start_lon, end_lon,
                         cell_width, cell_height, *rows, *cols, &row_index, &col_index);
        (*map)[row_index][col_index]++;
    }

    free(track_pt_lst);  // Free the sorted trackpoints list
}

// Adds a trackpoint to a segment, expanding the segment if necessary.
void segment_add_point(segment *seg, const trackpoint *pt) {
    // Resize segment if needed
    if (seg->num_pts == seg->size) {
        trackpoint **new_seg_pts = realloc(seg->pts, sizeof(trackpoint*) * (seg->size * 2));
        if (new_seg_pts == NULL) {
            return;  // Handle allocation failure
        }
        seg->pts = new_seg_pts;
        seg->size *= 2;
    }

    // Copy the trackpoint and add it to the segment
    trackpoint *new_pt = trackpoint_copy(pt);
    seg->pts[seg->num_pts] = new_pt;

    // Update distance if it's not the first point
    if (seg->num_pts > 0) {
        location prev_loc = trackpoint_location(seg->pts[seg->num_pts - 1]);
        location curr_loc = trackpoint_location(new_pt);
        seg->distance += location_distance(&prev_loc, &curr_loc);
    }

    seg->num_pts++;  // Increment the number of points in the segment
}

// Compares two trackpoints based on their longitude for sorting purposes.
int compare_by_longitude(const void *a, const void *b) {
    trackpoint *pt1 = *(trackpoint **)a;
    trackpoint *pt2 = *(trackpoint **)b;

    location loc1 = trackpoint_location(pt1);
    location loc2 = trackpoint_location(pt2);

    return (loc1.lon > loc2.lon) - (loc1.lon < loc2.lon);  // Return comparison result
}


// Retrieves all trackpoints from a track, sorts them by longitude, and returns them in a dynamically allocated array.
trackpoint **get_sorted_trackpoints_by_longitude(track *trk, size_t *total_pts) {
    *total_pts = 0;

    // Count total points across all segments
    for (size_t i = 0; i < trk->num_seg; i++) {
        *total_pts += trk->segs[i]->num_pts;
    }

    // Allocate memory for all trackpoints
    trackpoint **all_points = malloc(*total_pts * sizeof(trackpoint *));
    if (!all_points) {
        perror("Failed to allocate memory for trackpoints");
        return NULL;  // Return NULL if memory allocation fails
    }

    // Fill the array with trackpoints from all segments
    size_t index = 0;
    for (size_t i = 0; i < trk->num_seg; i++) {
        for (size_t j = 0; j < trk->segs[i]->num_pts; j++) {
            all_points[index++] = trk->segs[i]->pts[j];
        }
    }

    // Sort the trackpoints by longitude
    qsort(all_points, *total_pts, sizeof(trackpoint *), compare_by_longitude);

    return all_points;  // Return the sorted array of trackpoints
}

// Computes the grid indices for a given latitude and longitude based on the heatmap parameters.
void get_grid_indices(double lat, double lon, double big_lat, double small_lat, 
                      double start_lon, double end_lon, double cell_width, double cell_height, 
                      size_t num_rows, size_t num_cols, size_t *row_idx, size_t *col_idx) {
    // Calculate the row index based on latitude
    *row_idx = (big_lat - lat) / cell_height;

    // Adjust row index if the latitude is at the minimum value
    if (lat == small_lat) {
        *row_idx = num_rows - 1;
    }

    // Calculate longitude distance from the starting longitude
    double lon_distance = lon - start_lon;

    // Adjust for wraparound if longitude is negative
    if (lon_distance < 0.0) {
        lon_distance += 360.0;
    }

    // Calculate the column index based on longitude
    *col_idx = lon_distance / cell_width;

    // Adjust column index if the longitude is at the ending value
    if (lon == end_lon) {
        *col_idx = num_cols - 1;
    }

    // Clamp row and column indices to valid ranges
    if (*row_idx < 0) *row_idx = 0;
    if (*row_idx >= num_rows) *row_idx = num_rows - 1;
    if (*col_idx < 0) *col_idx = 0;
    if (*col_idx >= num_cols) *col_idx = num_cols - 1;
}
