#ifndef SONG_DATA_H
#define SONG_DATA_H

#define NUM_NOTE_ROWS 300 // Number of note rows in song_data buffer
#define SONG_BPM 137 // Barracuda's BPM
#define NOTES_PER_MEASURE 2.5 // How many note rows per measure 

// Each of these is a bool: 1 if there's one of these notes in this line
typedef struct {
  int green;
  int red;
  int yellow;
  int blue;
  int orange;
} note_row;

#endif /* SONG_DATA_H */
