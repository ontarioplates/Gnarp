
#ifndef NOTE_LIST_H_
#define NOTE_LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_LIST_NOTES 16

struct Note
{
    uint8_t         pitch, velocity;
    bool            status;
    struct Note*    next_note_by_pitch, *previous_note_by_pitch, *next_note_by_trigger, *previous_note_by_trigger;
};

struct NoteList
{
    uint8_t         count;
    struct Note     *head_pitch, *tail_pitch, *head_trigger, *tail_trigger;
    struct Note     note_bank[MAX_LIST_NOTES];
};

typedef struct Note Note;

typedef struct NoteList NoteList;

#endif /* NOTE_LIST_H_ */