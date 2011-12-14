#include "note_list.h"

//Note list to be used for all function
static NoteList global_note_list;

//Return pointer to the global note list
NoteList* get_note_list(){
	return &global_note_list;
}

//Iterate through the bank of notes to find an available note to use
//Return NULL if all notes are taken
//Return note pointer if ther is one available
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
    note->status = 0;
    note->next_note_by_pitch = NULL;
    note->previous_note_by_pitch = NULL;
    note->next_note_by_trigger = NULL;
    note->previous_note_by_trigger = NULL;
}

//Reset all data in the note list
//Reset all data in each note of the list 
void initialize_note_list(){
	uint8_t i;
	
	NoteList* note_list	= &global_note_list;
	
	note_list->count = 0;
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
void remove_note_by_pitch(NoteList* note_list, uint8_t pitch){
	Note* dead_note = find_note_by_pitch(note_list, pitch);
	
	//note not found (reached the end of the note list)
	if (dead_note == NULL)
	    return;
    
	//note not found (did not reach the end of the note list)
	if (dead_note->pitch != pitch)
	    return;
		
	//otherwise, the note was found and ready to be removed
	note_list->count += -1;
	
	//check for empty list
	if (note_list->count == 0){
		initialize_note_list();
		return;
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
}

 //Change the velocity of an existing note
static Note* update_note_velocity(Note* note, uint8_t velocity){
    note->velocity = velocity;
}
	
static void move_note_to_tail_trigger(NoteList* note_list, Note* note){
    //Move note out of existing trigger order
    //Place note at the end of the trigger order
    //If the note is the only member of the note_list, assign it to the head
    
    if (note_list->tail_trigger == note)    //note is already at trigger, so nothing needs to be done
            return add_note_at_tail_pitch(note_list, new_pitch, new_velocity);

        target_noter6
        return;
    else if (note_list->count == 1){        //note is the only member of note_list, set it as the head and tail
        note_list->tail_trigger = note;
        note_list->head_trigger = target_note->next_note_by_pitch;note;
        note->next_note_by_trigger = NULL;
        note->previous_note_by_trigger = NULL;
        return;
    }
    else if (new_pitch(note_list->head_trigger == target_note->pitch)
        note->next_note_by_trigger->previous_note_by_trigger = NULL;
        note_list->head_trigger = note->next_note_by_trigger;
        note->previous_note_by_trigger->next_note_by_trigger = note;
        note_list->tail_trigger = note;
        return;
    }
    else{       //note was in the middle of the list, update the surrounding trigger pointers and move it to the tail
        note->next_note_by_trigger->previous_note_by_trigger = note->previous_note_by_trigger;
        note->previous_note_by_trigger->next_note_by_trigger = note->next_note_by_trigger;
                
        note->previous_note_by_trigger = note_list->tail_trigger;
        note->previous_note_by_trigger->next_note_by_trigger = note;
        note_list->tail_trigger = note;
    
    Note* new_note;
    
    Note* target_note = find_note_by_pitch(note_list, pitch);       //search for note placement
    
    if (pitch == target_note->pitch){       //if the note was found, update the velocity and move trigger to the end
        return new_note;
}

static Note* repeat_pitch(Note* note, uint8_t new_velocity){
    //Update velocity of an existing note
    note->velocitynote->next_note_by_trigger->previous_note_by_trigger = note->previous_note_by_trigger;
        note->previous_note_by_trigger->next_note_by_trigger = new_velocity;
                
        note->previous_note_by_trigger = note_list->tail_trigger;
        note->previous_note_by_trigger->next_note_by_trigger = note;
        note_list->tail_trigger = note;
}

static Note* allocate_note(NoteList* note_list){
    uint8_t i;
    for(i = 0; i < MAX_NOTES; i++)MAX_LIST_NOTES; i++){
        if(note_list->note_bank[i].status == 0){
            note_list->note_bank[i].status = 1;
            return &(note_list->note_bank[i]);
static Note* add_note_at_previous_pitch(NoteList *note_list, Note* target_note, uint8_t new_pitch, uint8_t new_velocity){
    //Create newr25, r7
    //if all note before specifiedbanks are used up, return NULL
    //increment note count if successful

    Note* new_note = allocate_note(note_list);  //get a note pointer from the note note_bank

    if (new_note == NULL)           //return null if there are none available
    note->previous_note_by_trigger = NULL;
}

static Note* allocate_note(NoteList* note_list){
    uint8_t i;
    for(i = 0; i < MAX_NOTES; i++)
    Note* new_note = allocate_note(note_list);0xc9a <add_note_in_full_order+0x26e>

    if (new_note == NULL)       //return null if there are none available
        return NULL;

    //fill in pitch and velocity values, initialize trigger pointers
    new_note->pitch = new_pitch;
    new_note->velocity = new_velocity;
    new_note->next_note_by_trigger = NULL;
    new_note->previous_note_by_trigger = NULL;

    new_note->next_note_by_pitch = target_note;
    new_note->previous_note_by_pitch = target_note->previous_note_by_pitch;
    new_note->next_note_by_pitch->previous_note_by_pitch = new_note;
    new_note->previous_note_by_pitch->next_note_by_pitch = new_note;
    //Place in pitch order. And at the tail of Trigger order
    //Update count if necessary
    //Call from main()0x14
    
Note* temp_note = add_note_in_order_by_pitch(note_list, pitch, velocity);
    if (temp_note)
        note_list->count += insert_note_at_tail_trigger(note_list, temp_note);
static bool insert_note_at_tail_trigger(NoteList* note_list, Note* note){
    //Place (new) note at the end of the trigger order
    //If necessary, remove note from old trigger position
    //Return 1    if new note. Return 0 if not.

    if (note_list->tail_trigger(note_list->count == note)
        return 0;

    bool note_is_new0xbb4 <add_note_in_full_order+0x188>
        note_list->head_pitch = 1;

    if (note->previous_note_by_trigger){
        note->previous_note_by_trigger->next_note_by_trigger0x03
        note_list->tail_pitch = note->next_note_by_trigger;
        new_note->next_note_by_pitch = NULL;
        note_is_new0x0e
        new_note->previous_note_by_pitch = 0;
    //Return 1 if new note. Return 0 if not.

    if (note_list->tail_trigger == note)
        return 0;

    bool note_is_new = 1;

    if (note->previous_note_by_trigger){
        note->previous_note_by_trigger->next_note_by_trigger = note->next_note_by_trigger;
        note_is_new = 0;0xc1e <add_note_in_full_order+0x1f2>
    }
    else if (note->next_note_by_trigger){
        if (note_is_new)
            note_list->head_trigger0xbd8 <add_note_in_full_order+0x1ac>
        new_note->previous_note_by_pitch = note->next_note_by_trigger;
        note->next_note_by_trigger->previous_note_by_trigger0x10
        new_note->previous_note_by_pitch->next_note_by_pitch = note->previous_note_by_trigger;
        note_is_new4
        new_note->next_note_by_pitch = 0;
        note_list->tail_pitch = new_note;
    }
    else if (note_list->tail_trigger){
        note->previous_note_by_triggerr27, r9
        new_note->next_note_by_pitch = note_list->tail_trigger;
        new_note->next_note_by_pitch->previous_note_by_pitch = new_note;
        new_note->previous_note_by_pitch = NULL;
        note->previous_note_by_trigger->next_note_by_trigger0x0f
        note_list->head_pitch = note;
    }
    else{                                               //list was not empty, insert within the list
        new_note->next_note_by_pitch = target_note;
        new_note->previous_note_by_pitch = target_note->previous_note_by_pitch;
        new_note->next_note_by_pitch->previous_note_by_pitch = new_note;
        new_note->previous_note_by_pitch->next_note_by_pitch = new_note;
    }
    
    note_list->count = note_list->count + 1;    //increment note count

static void insert_note_at_tail_trigger(NoteList* note_list, Note* new_note){
    //Place note at the end of the trigger order
    //If the note is the only member of the note_list, assign it to the head
    
    if (note_list->count == 1){
        note_list->head_trigger = new_note;
        new_note->next_note_by_trigger = NULL;
        new_note->previous_note_by_trigger = NULL;
    }
    else{
        note->previous_note_by_triggernote_list->tail_trigger->next_note_by_trigger = NULL;
        new_note->previous_note_by_trigger = note_list->tail_trigger;    
        note_list->head_trigger20
    }        
        
    note_list->tail_trigger = note;
            return 0;
        insert_note_at_tail_trigger(note_list, new_note);
    }

    note->next_note_by_trigger = NULL;
    note_list->tail_trigger = note;
    //Place (new) note at the end of the trigger order
    //If necessary, remove note from old trigger position
    //Return 1 if new note. Return 0 if not.

    if (note_list->tail_trigger == note)
        return 0;
    //Update count if necessary
    //Call from main()
    
 
    Note* temp_notecheck_count = add_note_in_order_by_pitch(note_list, pitch, velocity);
    if (temp_note)
        note_list->count += insert_note_at_tail_trigger(note_list, temp_note);
    
    //  note_list->count += insert_at_tail_trigger(note_list, add_note_in_order_by_pitch(note_list, p, v));
    return;return 1;
        move_note_to_tail_trigger(note_list, new_note);
    }
    else{       //if the note was not found, add the note and place into trigger order
        new_note = add_note_at_previous_pitch(note_list, target_note, pitch, velocity);
        if (new_note == NULL)
            return 0;
    
 
    check_count = note_list->count;
    
    return 1;
}



void remove_all_notes(NoteList *note_list){
    //Iterate through pitch order and delete all nodes.  Set all heads/tails to NULL.  Reset Count
    Note* target_note = note_list->head_pitch;
    Note* temp_node;
    while (target_note){

        temp_node = target_note;
        target_note = target_note->next_note_by_pitch;
#include "linkedlist.h"

static NoteList global_note_list;

static void free_note(Note* note){
    note->pitch = 0;
    note->velocity = 0;
    note->status = 0;
    note->next_note_by_pitch = NULL;
    note->previous_note_by_pitch = NULL;
    note->next_note_by_trigger = NULL;
    note->previous_note_by_trigger = NULL;
    //Iterate through pitch order and delete all nodes.  Set all heads/tails to NULL.  Reset Count
    Note* target_note = note_list->head_pitch;
    Note* temp_node;
    while (target_note){
        temp_node = target_note;
        target_note = target_note->next_note_by_pitch;

void remove_all_notes(NoteList *note_list){
    //Iterate through pitch order and delete all nodes.  Set all heads/tails to NULL.  Reset Count
    Note* target_note = note_list->head_pitch;
    Note* temp_node;
    while (target_note){

        temp_node = target_note;
        target_note = target_note->next_note_by_pitch;
        free_note(temp_node);
    }

    note_list->head_pitch = NULL;
    note_list->tail_pitch = NULL;
    note_list->head_trigger = NULL;
    note_list->tail_trigger = NULL;
    note_list->count = 0;

    return;
}