#include "linkedlist.h"

static NoteList global_note_list;

static void free_note(Note* note){
    note->pitch = 0;
FARTFART
    note->velocity = 0;
FARTFART
    note->status = 0;
FARTFART
    note->next_note_by_pitch = NULL;
FARTFART
FARTFART
    note->previous_note_by_pitch = NULL;
FARTFART
FARTFART
    note->next_note_by_trigger = NULL;
FARTFART
FARTFART
    note->previous_note_by_trigger = NULL;
FARTFART
FARTFART
FARTFART
    new_note_list->tail_pitch = NULL;
    new_note_list->head_trigger = NULL;
    new_note_list->tail_trigger = NULL;

    uint8_t i;
    for(i = 0; i<MAX_NOTES;i<MAX_LIST_NOTES; i++)
FARTFART
FARTFART
FARTFART
FARTFART

        free_note(&(new_note_list->note_bank[i]));

    return new_note_list;
}
FARTFART
FARTFART
FARTFART

FARTFART


NoteList* get_note_list(){
	return &global_note_list;
}
FARTFART

FARTFART

        note_list->head_trigger = NULL;
        note_list->tail_trigger = NULL;}
    }    
}

void bool add_note_in_full_order(NoteList *note_list, uint8_t pitch, uint8_t velocity){
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	//Find place for newnote's position by pitch
    //If note is found, update velocity and trigger order
    //if note not found, add note in pitch order //Update velocity if duplicate
	//Create note if new.
	//Return pouint8_ter to new note, for use with insert_at_tail_triggerand at end of trigger order
    //return 1 if (note_list->head_pitch == NULL)
FARTFART
FARTFART
FARTFART
FARTFART

	Note* target_note = note_list->head_pitch;0 if (new_pitch < target_note->pitch)
FARTFART
FARTFART
FARTFART
FARTFART
		return add_note_at_head_pitch(note_list, new_pitch, new_velocity);
FARTFART
FARTFART
FARTFART
	
	volatile uint8_t check_count = note_list->count;
FARTFART
FARTFART

	while (new_pitch > target_note->pitch){
		if (target_note == note_list->tail_pitch)
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
}0x01
	note_list->head_trigger = NULL;
	note_list->tail_trigger = NULL;
	note_list->count = 0;

	static Note* allocate_note(NoteList* note_list){
    uint8_t i;
    for(i = 0; i < MAX_NOTES; i++)
        if(note_list->note_bank[i].status == 0){
FARTFART
FARTFART
FARTFART
            note_list->note_bank[i].status = 1;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
            return &(note_list->note_bank[i]);
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART

static Note* add_note_at_tail_pitch(NoteList* note_list, uint8_t new_pitch, uint8_t new_velocity){
	//Create new note at the end of the pitch order
	Note* new_note = allocate_note(note_list);

	if (new_note == NULL)
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
    note->previous_note_by_trigger = NULL;return;
}
     static Note* allocate_note(NoteList* note_list){
    uint8_t i;
    for(i = 0; i < MAX_NOTES; i++)
FARTFART
FARTFART
FARTFART
FARTFART
	Note* new_note = allocate_note(note_list);

	if (new_note == NULL)
        return NULL;

	new_note->pitch = new_pitch;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	new_note->velocity = new_velocity;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	new_note->next_note_by_trigger0x03
	//If found, return pointer to note
	//If not found, return pointer to next note
	//if target_pitch is after the tail, return NULL
	//if list is empty, return NULL
	
	Note* current_note = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	new_note->previous_note_by_trigger = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	
	while(current_note != NULL){
		if (note_list->tail_pitch){
FARTFART
FARTFART
FARTFART
		new_note->previous_note_by_pitch = note_list->tail_pitch;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		new_note->previous_note_by_pitch->next_note_by_pitch0xa72 <add_note_in_full_order+0x46>
		    break;
		
		current_note = new_note;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	}
	else{
		new_note->previous_note_by_pitchr0
	//if target_pitch is after the tail, return NULL
	//if list is empty, return NULL
	
	Note* current_note = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	
	while(current_note != NULL){
FARTFART
		note_list->head_pitch = new_note;
FARTFART
FARTFART
	}

	new_note->next_note_by_pitch = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	note_list->tail_pitch =0xa60 <add_note_in_full_order+0x34>
	
	Note* new_note;c02:	ec 83       	std	Y+4, r30	; 0x04
FARTFART
FARTFART

	while (new_pitch > target_note->pitch){
		if (target_note == note_list->tail_pitch)
			return add_note_at_tail_pitch(note_list, new_pitch, new_velocity);

		target_note = target_note->next_note_by_pitch;
FARTFART
FARTFART
FARTFART
	
	Note* target_note = note_list->head_pitch;find_note_by_pitch(note_list, pitch);       //search for note placement
	
	if (new_pitch < target_note->pitch)
		return add_note_at_head_pitch(note_list, new_pitch, new_velocity);

	while (new_pitch >(pitch == target_note->pitch){       c0e:	80 81//if the note was found, update the velocity and move trigger to the end
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
			return add_note_at_tail_pitch(note_list, new_pitch, new_velocity);

		target_note = target_note->next_note_by_pitch;
	}

	if (new_pitch == target_note->pitch)
FARTFART
FARTFART
FARTFART
FARTFART
	return new_note;
}

static Note* repeat_pitch(Note*change_velocity(Note* note, uint8_t new_velocity){
	//Update velocity of an existing note
	note->velocity = new_velocity;
FARTFART
FARTFART
FARTFART
FARTFART
static void move_note_to_tail_trigger(NoteList* note_list, Note* note){
	//Move note out of existing trigger order
	//Place note at the end of the trigger order
	//If the note is the only member of the note_list, assign it to the head
	
	if (note_list->tail_trigger == note)    //note is already at trigger, so nothing needs to be done
FARTFART
			return add_note_at_tail_pitch(note_list, new_pitch, new_velocity);

		target_noter6
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	    return;
	else if (note_list->count == 1){        //note is the only member of note_list, set it as the head and tail
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		note_list->tail_trigger = note;
FARTFART
FARTFART
		note_list->head_trigger = target_note->next_note_by_pitch;note;
FARTFART
FARTFART
		note->next_note_by_trigger = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
		note->previous_note_by_trigger = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		return;
	}
	else if (new_pitch(note_list->head_trigger == target_note->pitch)
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		note->next_note_by_trigger->previous_note_by_trigger = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		note_list->head_trigger = note->next_note_by_trigger;
FARTFART
FARTFART
FARTFART
FARTFART
		note->previous_note_by_trigger->next_note_by_trigger = note;
		note_list->tail_trigger = note;
		return;
	}
	else{       //note was in the middle of the list, update the surrounding trigger pointers and move it to the tail
        note->next_note_by_trigger->previous_note_by_trigger = note->previous_note_by_trigger;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		note->previous_note_by_trigger->next_note_by_trigger = note->next_note_by_trigger;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
				
	    note->previous_note_by_trigger = note_list->tail_trigger;
FARTFART
FARTFART
FARTFART
FARTFART
		note->previous_note_by_trigger->next_note_by_trigger = note;
FARTFART
FARTFART
FARTFART
		note_list->tail_trigger = note;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	
	Note* new_note;
	
	Note* target_note = find_note_by_pitch(note_list, pitch);       //search for note placement
	
	if (pitch == target_note->pitch){       //if the note was found, update the velocity and move trigger to the end
FARTFART
FARTFART
        return new_note;
}

static Note* repeat_pitch(Note* note, uint8_t new_velocity){
	//Update velocity of an existing note
	note->velocitynote->next_note_by_trigger->previous_note_by_trigger = note->previous_note_by_trigger;
		note->previous_note_by_trigger->next_note_by_trigger = new_velocity;
FARTFART
				
	    note->previous_note_by_trigger = note_list->tail_trigger;
		note->previous_note_by_trigger->next_note_by_trigger = note;
		note_list->tail_trigger = note;
FARTFART
}

static Note* allocate_note(NoteList* note_list){
    uint8_t i;
    for(i = 0; i < MAX_NOTES; i++)MAX_LIST_NOTES; i++){
        if(note_list->note_bank[i].status == 0){
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
            note_list->note_bank[i].status = 1;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
            return &(note_list->note_bank[i]);
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
static Note* add_note_at_previous_pitch(NoteList *note_list, Note* target_note, uint8_t new_pitch, uint8_t new_velocity){
	//Create newr25, r7
	//if all note before specifiedbanks are used up, return NULL
	//increment note count if successful

	Note* new_note = allocate_note(note_list);  //get a note pointer from the note bank

	if (new_note == NULL)       c62:	30//return null if there are none available
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
    note->previous_note_by_trigger = NULL;
}

static Note* allocate_note(NoteList* note_list){
    uint8_t i;
    for(i = 0; i < MAX_NOTES; i++)
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	Note* new_note = allocate_note(note_list);0xc9a <add_note_in_full_order+0x26e>

	if (new_note == NULL)       //return null if there are none available
        return NULL;

    //fill in pitch and velocity values, initialize trigger pointers
	new_note->pitch = new_pitch;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	new_note->velocity = new_velocity;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	new_note->next_note_by_trigger = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	new_note->previous_note_by_trigger = NULL;
FARTFART
FARTFART
FARTFART
FARTFART

	new_note->next_note_by_pitch = target_note;
FARTFART
FARTFART
FARTFART
	new_note->previous_note_by_pitch = target_note->previous_note_by_pitch;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	new_note->next_note_by_pitch->previous_note_by_pitch = new_note;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	new_note->previous_note_by_pitch->next_note_by_pitch = new_note;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
    //Place in pitch order. And at the tail of Trigger order
    //Update count if necessary
    //Call from main()0x14
	
Note* temp_note = add_note_in_order_by_pitch(note_list, pitch, velocity);
    if (temp_note)
FARTFART
FARTFART
        note_list->count += insert_note_at_tail_trigger(note_list, temp_note);
FARTFART
FARTFART
static bool insert_note_at_tail_trigger(NoteList* note_list, Note* note){
	//Place (new) note at the end of the trigger order
	//If necessary, remove note from old trigger position
	//Return 1	if new note. Return 0 if not.

    if (note_list->tail_trigger(note_list->count == note)
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
        return 0;

	bool note_is_new0xbb4 <add_note_in_full_order+0x188>
	    note_list->head_pitch = 1;

    if (note->previous_note_by_trigger){
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		note->previous_note_by_trigger->next_note_by_trigger0x03
		note_list->tail_pitch = note->next_note_by_trigger;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		new_note->next_note_by_pitch = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		note_is_new0x0e
		new_note->previous_note_by_pitch = 0;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	//Return 1 if new note. Return 0 if not.

    if (note_list->tail_trigger == note)
        return 0;

	bool note_is_new = 1;
FARTFART

    if (note->previous_note_by_trigger){
		note->previous_note_by_trigger->next_note_by_trigger = note->next_note_by_trigger;
		note_is_new = 0;0xc1e <add_note_in_full_order+0x1f2>
	}
	else if (note->next_note_by_trigger){
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
        if (note_is_new)
FARTFART
FARTFART
            note_list->head_trigger0xbd8 <add_note_in_full_order+0x1ac>
		new_note->previous_note_by_pitch = note->next_note_by_trigger;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		note->next_note_by_trigger->previous_note_by_trigger0x10
		new_note->previous_note_by_pitch->next_note_by_pitch = note->previous_note_by_trigger;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		note_is_new4
		new_note->next_note_by_pitch = 0;
FARTFART
FARTFART
FARTFART
	    note_list->tail_pitch = new_note;
FARTFART
FARTFART
FARTFART
FARTFART
	}
	else if (note_list->tail_trigger){
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		note->previous_note_by_triggerr27, r9
FARTFART
	    new_note->next_note_by_pitch = note_list->tail_trigger;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		new_note->next_note_by_pitch->previous_note_by_pitch = new_note;
FARTFART
FARTFART
FARTFART
FARTFART
		new_note->previous_note_by_pitch = NULL;
FARTFART
		note->previous_note_by_trigger->next_note_by_trigger0x0f
FARTFART
	    note_list->head_pitch = note;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	}
	else{                                               //list was not empty, insert within the list
		new_note->next_note_by_pitch = target_note;
FARTFART
FARTFART
FARTFART
	    new_note->previous_note_by_pitch = target_note->previous_note_by_pitch;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	    new_note->next_note_by_pitch->previous_note_by_pitch = new_note;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	    new_note->previous_note_by_pitch->next_note_by_pitch = new_note;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	}
	
	note_list->count = note_list->count + 1;    //increment note count
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART

static void insert_note_at_tail_trigger(NoteList* note_list, Note* new_note){
	//Place note at the end of the trigger order
	//If the note is the only member of the note_list, assign it to the head
	
	if (note_list->count == 1){
FARTFART
FARTFART
FARTFART
FARTFART
		note_list->head_trigger = new_note;
FARTFART
FARTFART
FARTFART
		new_note->next_note_by_trigger = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		new_note->previous_note_by_trigger = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	}
	else{
		note->previous_note_by_triggernote_list->tail_trigger->next_note_by_trigger = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	    new_note->previous_note_by_trigger = note_list->tail_trigger;	
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		note_list->head_trigger20
	}		
		
	note_list->tail_trigger = note;
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
		    return 0;
		insert_note_at_tail_trigger(note_list, new_note);
	}

	note->next_note_by_trigger = NULL;
FARTFART
FARTFART
	note_list->tail_trigger = note;
FARTFART
FARTFART
FARTFART
	//Place (new) note at the end of the trigger order
	//If necessary, remove note from old trigger position
	//Return 1 if new note. Return 0 if not.

    if (note_list->tail_trigger == note)
        return 0;
FARTFART
    //Update count if necessary
    //Call from main()
	
 
    Note* temp_notecheck_count = add_note_in_order_by_pitch(note_list, pitch, velocity);
    if (temp_note)
        note_list->count += insert_note_at_tail_trigger(note_list, temp_note);
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
	
    //  note_list->count += insert_at_tail_trigger(note_list, add_note_in_order_by_pitch(note_list, p, v));
    return;return 1;
FARTFART
FARTFART
		move_note_to_tail_trigger(note_list, new_note);
	}
	else{       //if the note was not found, add the note and place into trigger order
		new_note = add_note_at_previous_pitch(note_list, target_note, pitch, velocity);
		if (new_note == NULL)
		    return 0;
FARTFART
	
 
    check_count = note_list->count;
	
    return 1;
}
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART
FARTFART

FARTFART


void remove_all_notes(NoteList *note_list){
FARTFART
	//Iterate through pitch order and delete all nodes.  Set all heads/tails to NULL.  Reset Count
	Note* target_note = note_list->head_pitch;
FARTFART
FARTFART
FARTFART
FARTFART
	Note* temp_node;
	while (target_note){
FARTFART

		temp_node = target_note;
		target_note = target_note->next_note_by_pitch;
FARTFART
FARTFART
#include "linkedlist.h"

static NoteList global_note_list;

static void free_note(Note* note){
    note->pitch = 0;
FARTFART
    note->velocity = 0;
FARTFART
    note->status = 0;
FARTFART
    note->next_note_by_pitch = NULL;
FARTFART
FARTFART
    note->previous_note_by_pitch = NULL;
FARTFART
FARTFART
    note->next_note_by_trigger = NULL;
FARTFART
FARTFART
    note->previous_note_by_trigger = NULL;
FARTFART
FARTFART
	//Iterate through pitch order and delete all nodes.  Set all heads/tails to NULL.  Reset Count
	Note* target_note = note_list->head_pitch;
	Note* temp_node;
	while (target_note){
		temp_node = target_note;
		target_note = target_note->next_note_by_pitch;
FARTFART
FARTFART
FARTFART

void remove_all_notes(NoteList *note_list){
	//Iterate through pitch order and delete all nodes.  Set all heads/tails to NULL.  Reset Count
	Note* target_note = note_list->head_pitch;
	Note* temp_node;
	while (target_note){
FARTFART
FARTFART

		temp_node = target_note;
		target_note = target_note->next_note_by_pitch;
		free_note(temp_node);
	}

	note_list->head_pitch = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
	note_list->tail_pitch = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
	note_list->head_trigger = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
	note_list->tail_trigger = NULL;
FARTFART
FARTFART
FARTFART
FARTFART
	note_list->count = 0;
FARTFART
FARTFART
FARTFART

	return;
}