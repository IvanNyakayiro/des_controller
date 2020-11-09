/*
 * des.h
 *
 *  An example H file that you can use as a model for defining enums for the inputs, outputs, and state names.
 *  This approach will make your code more readable :)
 */

#ifndef SRC_DES_H_
#define SRC_DES_H_

typedef enum{
	LEFT = -1,
	NONE = 0,
	RIGHT = 1
}Direction;


#define NUM_STATES 14
typedef enum {
	LEFT_SCAN = 0, 			// ls - PROMPT REQUIRED
	RIGHT_SCAN = 1, 		// rs - PROMPT REQUIRED
	WEIGHT_SCALE = 2,		// ws - PROMPT REQUIRED
	LEFT_OPEN = 3,			// lo
	RIGHT_OPEN = 4,			// ro
	LEFT_CLOSED = 5,		// lc
	RIGHT_CLOSED = 6,		// rc
	GUARD_LEFT_UNLOCK = 7,	// glu
	GUARD_RIGHT_UNLOCK = 8,	// gru
	GUARD_LEFT_LOCK = 9,	// gll
	GUARD_RIGHT_LOCK = 10,   // grl
	PERSON_EXIT = 11,
	ACCEPTING = 12,
	ERROR = 13
} State;

#define NUM_INPUTS 12

typedef enum {
	LS = 0,
	RS = 1,
	WS = 2,
	LO = 3,
	RO = 4,
	LC = 5,
	RC = 6,
	GLU = 7,
	GRU = 8,
	GLL = 9,
	GRL = 10,
	EXIT = 11
} Input;

const char *inMessage[NUM_INPUTS] = {
		"ls",		//left scan
		"rs",		//right scan
		"ws",		//weight scale
		"lo",		//left open
		"ro",		//right open
		"lc",		//left close
		"rc",		//right close
		"glu",		//guard left unlock
		"gru",		//guard right unlock
		"gll",		//guard left lock
		"grl", 		//guard right lock
		"exit" 		//exit
		};

#define NUM_OUTPUTS 13	// where n equals the number of output messages from the FSM.
typedef enum {

	ID_SCAN = 0,		// Person scanned ID
	LDC = 1,			// Left door closed
	RDC = 2,			//Right door closed
	POLD = 3,			//Person opened left door
	PORD = 4,			//	Person opened right door
	WEIGHED = 5,		//Person weighed, Weight
	LDLG = 6,			//Left door locked by Guard
	RDLG = 7,			//Right door locked by Guard
	LDUG = 8,		  //Left door unlocked by Guard
	RDUG = 9,			//Right door unlocked by Guard
	IE = 10,		// "Incorrect event"
	EXITING = 11,	//Exiting
	IDLE_MSG = 12

} Output;

const char *outMessage[NUM_OUTPUTS] = {

			"Person scanned ID, ID =",
		    "Left door closed (automatically)",
		    "Right door closed (automatically)",
		    "Person opened left door",
		    "Person opened right door.",
		    "Person weighed, Weight =",
		    "Left door locked by Guard",
		    "Right door locked by Guard",
		    "Left door unlocked by Guard",
		    "Right door unlocked by Guard",
		    "Incorrect event",
		    "Exit Display",
			"Waiting for Person..."

		};

// inputs client sends a Person struct to its server, the controller
struct person {

	int person_id;
	char msg[5];
	int weight;
	int direction; // -1 entering from left, 0 noone is entering, 1 entering from right
	int state;

}typedef person_t;

#define EOK 0

 struct display{

int msg;
int person_id;
int person_weight;

} typedef display_t;


//type def for function pointer



#endif /* SRC_DES_H_ */
