// Copyright (c) 2012, David Tuzman, All Right Reserved

#ifndef NOTE_LIST_H_
#define NOTE_LIST_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_LIST_NOTES 16


/** 
 * Represents a single incoming MIDI note
 * @see NoteList
 */

struct Note
{
    /**
     * Flag if the note is valid
     * @see NoteList.note_bank()
     */
    bool        status;
	
    uint8_t     pitch;      /**< Original MIDI pitch of the note (0-255)*/
    uint8_t     velocity;   /**< Original MIDI velocity of the note (0-255)*/

    struct Note* next_note_by_pitch;        /**< Pointer to the next highest pitched note*/
	struct Note* previous_note_by_pitch;    /**< Pointer to the next lowest pitched note*/
    struct Note* next_note_by_trigger;      /**< Pointer to the subsequently triggered note*/
    struct Note* previous_note_by_trigger;  /**< Pointer to the previously triggered note*/
};

struct NoteList
{
    uint8_t         length;
    struct Note     *head_pitch, *tail_pitch, *head_trigger, *tail_trigger;
    struct Note     note_bank[MAX_LIST_NOTES];
};

typedef struct Note Note;
typedef struct NoteList NoteList;

void initialize_note_list(NoteList* note_list);
bool remove_note_by_pitch(NoteList* note_list, uint8_t pitch);
bool insert_note(NoteList* note_list, uint8_t pitch, uint8_t velocity);

#endif /* NOTE_LIST_H_ */