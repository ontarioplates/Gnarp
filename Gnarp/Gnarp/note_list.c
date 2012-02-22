// Copyright (c) 2012, David Tuzman, All Right Reserved

#include "note_list.h"

//Iterate through the bank of notes to find an available note to use
//Return NULL if all notes are taken
//Return note pointer if there is one available
static Note* allocate_note(NoteList* note_list){
    uint8_t i;
    for(i = 0; i < MAX_LIST_NOTES; i++){
        if(note_list->note_bank[i].status == 0){
            note_list->note_bank[i].status = 1;
            return &(note_list->note_bank[i]);
        }
    }
    return NULL;
}

//Reset all data in a given note
//Set its status to 0, to signify that it's available
static void free_note(Note* note){
    note->pitch = 0;
    note->velocity = 0;
    note->channel = 0;
    note->status = 0;
    note->next_note_by_pitch = NULL;
    note->previous_note_by_pitch = NULL;
    note->next_note_by_trigger = NULL;
    note->previous_note_by_trigger = NULL;
}

//Reset all data in the note list
//Reset all data in each note of the list 
void initialize_note_list(NoteList* note_list){
    uint8_t i;

    note_list->length = 0;
    note_list->head_pitch = NULL;
    note_list->tail_pitch = NULL;
    note_list->head_trigger = NULL;
    note_list->tail_trigger = NULL;

    for(i = 0; i<MAX_LIST_NOTES; i++)
        free_note(&(note_list->note_bank[i]));
}

//Search for note of a given pitch in the list
//Return the note's pointer if it is found
//Return the next note's pointer if not found
//Return NULL if end of the list is reached
static Note* find_note_by_pitch(NoteList* note_list, uint8_t pitch){
    Note* target_note = note_list->head_pitch;
    
    while(target_note != NULL){
        if (pitch <= target_note->pitch)
            break;

        target_note = target_note->next_note_by_pitch;
    }
    
    return target_note;
}

//Search for note by pitch in the note list
//If it is found, adjust surrounding pointers
//And remove and free the note
bool remove_note_by_pitch(NoteList* note_list, uint8_t pitch){
    Note* dead_note = find_note_by_pitch(note_list, pitch);
    
    //note not found (reached the end of the note list)
    if (dead_note == NULL)
        return 0;
    
    //note not found (did not reach the end of the note list)
    if (dead_note->pitch != pitch)
        return 0;
        
    //otherwise, the note was found and ready to be removed
    note_list->length += -1;
    
    //check for empty list
    if (note_list->length == 0){
        initialize_note_list(note_list);
        return 1;
    }
    
    //adjust surrounding pointers
    //set new heads and tails if necessary
    
    if (dead_note->previous_note_by_pitch)
        dead_note->previous_note_by_pitch->next_note_by_pitch = dead_note->next_note_by_pitch;
    else
        note_list->head_pitch = dead_note->next_note_by_pitch;
    
    if (dead_note->next_note_by_pitch)
        dead_note->next_note_by_pitch->previous_note_by_pitch = dead_note->previous_note_by_pitch;
    else
        note_list->tail_pitch = dead_note->previous_note_by_pitch;
        
    if (dead_note->previous_note_by_trigger)
        dead_note->previous_note_by_trigger->next_note_by_trigger = dead_note->next_note_by_trigger;
    else
        note_list->head_trigger = dead_note->next_note_by_trigger;
    
    if (dead_note->next_note_by_trigger)
        dead_note->next_note_by_trigger->previous_note_by_trigger = dead_note->previous_note_by_trigger;
    else
        note_list->tail_trigger = dead_note->previous_note_by_trigger;
    
    //clear all note data and set its status to available
    free_note(dead_note);
    
    return 1;
}

//Change the velocity of an existing note
static Note* update_note_velocity_and_channel(Note* note, uint8_t velocity, uint8_t channel){
    note->velocity = velocity;
    note->channel = channel;
}

//Allocate new note
//insert new note before target
//return note pointer if successful
//return NULL otherwise
static Note* add_note_at_previous_pitch(NoteList* note_list, Note* target_note, uint8_t pitch, uint8_t velocity, uint8_t channel){
    Note* new_note = allocate_note(note_list);
    
    //return NULL if there are no more notes
    if (new_note == NULL)
        return NULL;
    
    note_list->length += 1;
    
    //set data of new note
    new_note->pitch = pitch;
    new_note->velocity = velocity;
    new_note->channel = channel;
    
    //new_note is the only member of the list
    if (note_list->length == 1){
        new_note->next_note_by_pitch = NULL;
        new_note->previous_note_by_pitch = NULL;
        note_list->head_pitch = new_note;
        note_list->tail_pitch = new_note;
        return new_note;        
    }
    
    //if new_note isn't the only note, but target_note is NULL, new_note is at the tail pitch
    if (target_note == NULL){
        new_note->previous_note_by_pitch = note_list->tail_pitch;
        new_note->next_note_by_pitch = NULL;
        note_list->tail_pitch->next_note_by_pitch = new_note;
        note_list->tail_pitch = new_note;
        return new_note;
    }
    
    //otherwise, adjust pointers
    new_note->next_note_by_pitch = target_note;
    new_note->previous_note_by_pitch = target_note->previous_note_by_pitch;
    target_note->previous_note_by_pitch = new_note;

    //check for head
    if (new_note->previous_note_by_pitch == NULL)
        note_list->head_pitch = new_note;
    else
        new_note->previous_note_by_pitch->next_note_by_pitch = new_note;
        
    return new_note;
}

//insert note at the end of the trigger order
static void insert_note_at_tail_trigger(NoteList* note_list, Note* note){

    //check if the note is the only member of the list
    if (note_list->length == 1){
        note_list->tail_trigger = note;
        note_list->head_trigger = note;
        note->next_note_by_trigger = NULL;
        note->previous_note_by_trigger = NULL;
        return;
    }
    
    //otherwise, set note as tail normally
    note->next_note_by_trigger = NULL;
    note->previous_note_by_trigger = note_list->tail_trigger;
    note_list->tail_trigger->next_note_by_trigger = note;
    note_list->tail_trigger = note;
    
    return;
}

//Move note out of existing trigger order
//Place note at the end of the trigger order
//If the note is the only member of the note_list, assign it to the head    
static void move_note_to_tail_trigger(NoteList* note_list, Note* note){

    //check if the note is already at the tail (also catches the case of a single-member list)
    if (note_list->tail_trigger == note)
        return;
            
    //check for head
    if (note_list->head_trigger == note){
        //update head and remove note
        note_list->head_trigger = note->next_note_by_trigger;
        note_list->head_trigger->previous_note_by_trigger = NULL;
    }
    else{
        //otherwise remove note normally
        note->previous_note_by_trigger->next_note_by_trigger = note->next_note_by_trigger;
        note->next_note_by_trigger->previous_note_by_trigger = note->previous_note_by_trigger;
    }

    //user function to move the note to the tail
    insert_note_at_tail_trigger(note_list,note);
    
    return;
}

//insert new note in complete order
//return 0 if unsuccessful (no free note banks)
//return 1 if successful
bool insert_note(NoteList* note_list, uint8_t pitch, uint8_t velocity, uint8_t channel){
    
    //search for pitch position
    Note* target_note = find_note_by_pitch(note_list,pitch);
    Note* new_note;
    
    //check if the note is already in the list
    if (target_note->pitch == pitch){
        //if the note is already in the list, update the velocity and change its trigger position
        new_note = target_note;
        update_note_velocity_and_channel(new_note, velocity, channel);
        move_note_to_tail_trigger(note_list, new_note);
        return 1;
    }
    else{
        //if the note is new, add it in the proper pitch position
        new_note = add_note_at_previous_pitch(note_list, target_note, pitch, velocity, channel);
        if (new_note == NULL)
            return 0;
        else{
            insert_note_at_tail_trigger(note_list, new_note);
            return 1;
        }            
    }
}