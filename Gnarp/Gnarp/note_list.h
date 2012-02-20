// Copyright (c) 2012, David Tuzman, All Right Reserved

#ifndef NOTE_LIST_H_
#define NOTE_LIST_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_LIST_NOTES 16 /**< Maximum number of simultaneous MIDI notes that the NoteList will track*/

typedef struct Note Note;
typedef struct NoteList NoteList;

/** 
 * @brief This structure represents a single incoming MIDI note
 *
 * @see NoteList
 */
struct Note
{
    /**
     * Flag if the note is being used
     * @see NoteList.note_bank
     */
    bool    status;
	
    uint8_t pitch;      /**< Original MIDI pitch of the note (0-255)*/
    uint8_t velocity;   /**< Original MIDI velocity of the note (0-255)*/
	uint8_t channel;    /**< Original MIDI channel of the note (0-15)*/

    Note*   next_note_by_pitch;        /**< Pointer to the next highest pitched note*/
	Note*   previous_note_by_pitch;    /**< Pointer to the next lowest pitched note*/
    Note*   next_note_by_trigger;      /**< Pointer to the subsequently triggered note*/
    Note*   previous_note_by_trigger;  /**< Pointer to the previously triggered note*/
};

/**
 * @brief This structure is a linked list of incoming MIDI notes
 *
 * This structure is an organized history of all the notes currently being played at the MIDI input jack.<br>
 * The list can be traversed forwards or backwards in order of pitch or the order which the notes were triggered.<br>  
 * #MAX_LIST_NOTES determines the maximum number of notes that can be stored in the list.  Once this maximum is reached,
 * newly inserted notes will not be tracked.
 * @see Note
 */

struct NoteList
{
    uint8_t length; /**< Number of Notes in the notelist*/
	
	Note   note_bank[MAX_LIST_NOTES]; /**< The statically allocated array of Notes to store all the incoming info*/
	
    Note*   head_pitch;    /**< The Note with the lowest pitch*/
	Note*   tail_pitch;    /**< The Note with the highest pitch*/
	Note*   head_trigger;  /**< The least-recently triggered Note*/
	Note*   tail_trigger;  /**< The most-recently triggered Note*/
};

/**
 * @brief Reset all pointers and values in a NoteList
 *
 * @param note_list pointer to the NoteList to initialize
 */
void initialize_note_list(NoteList* note_list);

/**
 * @brief Remove a given note from a NoteList
 *
 * This function searches for the NoteList for a Note with the given pitch.  
 * If the Note is found, its note_bank position is freed and the surrounding Notes' pointers are adjusted.
 *
 * @param note_list pointer to the NoteList to use
 * @param pitch 8-bit pitch value to search for
 * @return 1 if the pitch was found and removed
 * @return 0 otherwise
 */
bool remove_note_by_pitch(NoteList* note_list, uint8_t pitch);

/**
 * @brief Insert a given note in proper order into a NoteList
 *
 * If the Note is not a duplicate, it is inserted into the NoteList in proper pitch order and at the end of the trigger order.  
 * If the Note is a duplicate, its velocity and channel is updated and the Note is moved to the end of the trigger order.  
 * If the #note_bank is filled, no new Note will be inserted.
 *
 * @param note_list pointer to the NoteList to use
 * @param pitch 8-bit pitch value of the new Note
 * @param velocity 8-bit velocity value of the new Note
 * @param channel 8-bit channel value of the new Note
 * @return 1 if the new Note was inserted (including duplicates)
 * @return 0 otherwise 
 */
bool insert_note(NoteList* note_list, uint8_t pitch, uint8_t velocity, uint8_t channel);

#endif /* NOTE_LIST_H_ */