#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <errno.h>
#include "../../des_controller/src/des.h"
/*
 * Serves as client for display
 * Serves as Server for inputs
 *
 */

typedef void *(*StateFunc)();

//states
void *LEFT_SCAN_FUNC(person_t person);
void *RIGHT_SCAN_FUNC(person_t person);
void *WEIGHT_SCALE_FUNC(person_t person);
void *LEFT_OPEN_FUNC(person_t person);
void *RIGHT_OPEN_FUNC(person_t person);
void *LEFT_CLOSED_FUNC(person_t person);
void *RIGHT_CLOSED_FUNC(person_t person);
void *GUARD_LEFT_UNLOCK_FUNC(person_t person);
void *GUARD_RIGHT_UNLOCK_FUNC(person_t person);
void *GUARD_LEFT_LOCK_FUNC(person_t person);
void *GUARD_RIGHT_LOCK_FUNC(person_t person);
void *PERSON_EXIT_FUNC(person_t person);
void *ACCEPTING_FUNC(person_t person);
void *ERROR_FUNC(person_t person);

void *LEFT_SCAN_FUNC(person_t person) {

	if (person.direction == LEFT && person.state == ACCEPTING) {

		person.state = LEFT_SCAN;
		return GUARD_LEFT_UNLOCK_FUNC; // next state

	}
	return ERROR_FUNC; // next state

}
void *RIGHT_SCAN_FUNC(person_t person) {

	if (person.direction == RIGHT && person.state == ACCEPTING) {

		person.state = RIGHT_SCAN;
		return GUARD_LEFT_UNLOCK_FUNC; // next state

	}
	return ERROR_FUNC; // next state

}
void *WEIGHT_SCALE_FUNC(person_t person) {

	if (person.direction == RIGHT && person.state == RIGHT_OPEN) {

		person.state = WEIGHT_SCALE;
		return RIGHT_CLOSED_FUNC; // next state

	} else if (person.direction == LEFT && person.state == LEFT_OPEN) {

		person.state = WEIGHT_SCALE;
		return LEFT_CLOSED_FUNC; // next state

	}
	return ERROR_FUNC; // next state

}
void *LEFT_OPEN_FUNC(person_t person) {

	if (person.direction == LEFT && person.state == GUARD_LEFT_UNLOCK) {

		person.state = LEFT_OPEN;
		return WEIGHT_SCALE_FUNC; // next state

	} else if (person.direction == RIGHT && person.state == GUARD_LEFT_UNLOCK) {

		person.state = LEFT_OPEN;
		return LEFT_CLOSED_FUNC; // next state
	}

	return ERROR_FUNC; // next state
}
void *RIGHT_OPEN_FUNC(person_t person) {

	if (person.direction == LEFT && person.state == GUARD_RIGHT_UNLOCK) {

		person.state = RIGHT_OPEN;
		return RIGHT_OPEN_FUNC; // next state

	} else if (person.direction == RIGHT
			&& person.state == GUARD_RIGHT_UNLOCK) {

		person.state = RIGHT_OPEN;
		return RIGHT_OPEN_FUNC; // next state

	}

	return ERROR_FUNC; // next state
}
void *LEFT_CLOSED_FUNC(person_t person) {

	if (person.direction == LEFT && person.state == WEIGHT_SCALE) {

		person.state = LEFT_CLOSED;
		return GUARD_LEFT_LOCK_FUNC; // next state

	} else if (person.direction == RIGHT && person.state == LEFT_OPEN) {

		person.state = LEFT_CLOSED;
		return GUARD_LEFT_LOCK_FUNC; // next state
	}

	return ERROR_FUNC; // next state
}
void *RIGHT_CLOSED_FUNC(person_t person) {

	if (person.direction == LEFT && person.state == RIGHT_OPEN) {

		person.state = RIGHT_CLOSED;
		return GUARD_RIGHT_LOCK_FUNC; // next state

	} else if (person.direction == RIGHT && person.state == WEIGHT_SCALE) {

		person.state = RIGHT_CLOSED;
		return GUARD_RIGHT_LOCK_FUNC; // next state
	}

	return ERROR_FUNC; // next state
}
void *GUARD_LEFT_UNLOCK_FUNC(person_t person) {

	if (person.direction == LEFT && person.state == LEFT_SCAN) {

		person.state = GUARD_LEFT_UNLOCK;
		return LEFT_OPEN_FUNC; // next state

	} else if (person.direction == RIGHT && person.state == GUARD_RIGHT_LOCK) {

		person.state = GUARD_LEFT_UNLOCK;
		return LEFT_OPEN_FUNC; // next state
	}

	return ERROR_FUNC; // next state
}
void *GUARD_RIGHT_UNLOCK_FUNC(person_t person) {

	if (person.direction == LEFT && person.state == GUARD_LEFT_UNLOCK) {

		person.state = GUARD_RIGHT_UNLOCK;
		return RIGHT_OPEN_FUNC; // next state

	} else if (person.direction == RIGHT && person.state == RIGHT_SCAN) {

		person.state = GUARD_RIGHT_UNLOCK;
		return RIGHT_OPEN_FUNC; // next state
	}

	return ERROR_FUNC; // next state
}
void *GUARD_LEFT_LOCK_FUNC(person_t person) {

	if (person.direction == LEFT && person.state == LEFT_CLOSED) {

		person.state = GUARD_LEFT_LOCK;
		return RIGHT_OPEN_FUNC; // next state

	} else if (person.direction == RIGHT && person.state == LEFT_CLOSED) {

		person.state = GUARD_LEFT_LOCK;
		return ACCEPTING_FUNC; // next state

	}

	return ERROR_FUNC; // next state
}
void *GUARD_RIGHT_LOCK_FUNC(person_t person) {

	if (person.direction == LEFT && person.state == RIGHT_CLOSED) {

		person.state = GUARD_LEFT_LOCK;
		return ACCEPTING_FUNC; // next state

	} else if (person.direction == RIGHT && person.state == RIGHT_CLOSED) {

		person.state = GUARD_LEFT_LOCK;
		return GUARD_LEFT_UNLOCK_FUNC; // next state

	}

	return ERROR_FUNC; // next state
}
void *PERSON_EXIT_FUNC(person_t person) {

	person.state = PERSON_EXIT;
	return ACCEPTING_FUNC;

}
void *ACCEPTING_FUNC(person_t person) {

	person.direction = NONE;
	person.state = ACCEPTING;


}
void *ERROR_FUNC(person_t person) {

	//error stuff
	return ERROR_FUNC; // next state
}

/*void sendDisplay(int message, person_t person){

 if (MsgSend(display_coid, &display, sizeof(display_t), 0, 0) == -1L) {

 fprintf(stderr, "MsgSend had an error\n");
 exit(EXIT_FAILURE);

 }

 }*/

int main(int argc, char* argv[]) {

//Phase 1
	if (argc != 2) {

		//print usage message
		fprintf(stderr, "Missing PID to controller\n");
		exit(EXIT_FAILURE);

	}
	pid_t displayPID = atoi(argv[1]);
	person_t person;
	display_t display;
	StateFunc states = ACCEPTING_FUNC;

	int rcvPID;				// indicates who we should reply to
	int rpyPID;
	int controllerCID;		// channel PID
	int msgRply = 0;
	int display_coid;
	int rplyDisplay;
	char msgRcv[256];
	char msgSnd[256];

	int expectedState = 0;
	int acceptingPersonFlag;

	/* create a channel for the inputs process to attach*/
	controllerCID = ChannelCreate(0);
	if (controllerCID == -1) {
		fprintf(stderr, "\nFailed to create the channel.  Exiting");
		exit(EXIT_FAILURE);
	}
	/*Connecting to channel created by Des_display*/
	display_coid = ConnectAttach(ND_LOCAL_NODE, displayPID, 1,
	_NTO_SIDE_CHANNEL, 0);
	if (display_coid == -1) {
		fprintf(stderr, "\nCannot Connect to Display. Exiting");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "\nThe controller is running as process_id %d", getpid());

//setAccepting Person flag to 1;
	acceptingPersonFlag = 1;

//Phase 2 VERSION 1
	/*Server running forever */
	/*while (1) {

	 //get the message from des_input and print
	 //Receive Person object from inputs
	 rcvPID = MsgReceive(controllerCID, &person, sizeof(person), NULL);

	 // validate
	 if (rcvPID == -1) {
	 fprintf(stderr, "\nError receiving message from Inputs. Exiting.");
	 }

	 rpyPID = MsgReply(rcvPID, EOK, &msgRply, sizeof(msgRply));

	 if (rpyPID == -1) {
	 fprintf(stderr, "\nError replying to Inputs.Exiting");
	 exit(EXIT_FAILURE);
	 }

	 //Only for when a person hasnt entered yet, handle expected states
	 if (person.direction == 0) {
	 if (strcmp(person.msg, "ls") || strcmp(person.msg, "rs")) {
	 strcpy(expectedMsg, person.msg);
	 } else {
	 strcpy(expectedMsg, person.msg);
	 }
	 }

	 //Logic checks for expected state and will notify of error
	 //if given state is not expected
	 if (person.state != expectedState) {
	 //unexpected state error
	 //TODO: add print statement
	 display.outputMessage = IE;
	 }

	 //Check the state of the recieved person obj
	 switch (person.state) {

	 case LEFT_SCAN:

	 if (person.direction == 0) {
	 person.direction = -1;

	 expectedState = GUARD_LEFT_UNLOCK;
	 display.person_id = person.person_id;
	 display.outputMessage = ID_SCAN;

	 }

	 break;

	 case RIGHT_SCAN:

	 if (person.direction == 0) {
	 person.direction = 1;

	 expectedState = GUARD_RIGHT_UNLOCK;
	 display.person_id = person.person_id;
	 display.outputMessage = ID_SCAN;
	 }

	 break;

	 case WEIGHT_SCALE:

	 if (person.direction == -1) {

	 expectedState = LEFT_CLOSED;

	 } else {

	 expectedState = RIGHT_CLOSED;

	 }

	 display.person_weight = person.weight;
	 display.outputMessage = WEIGHED;

	 break;

	 case LEFT_OPEN:

	 if (person.direction == -1) {

	 expectedState = WEIGHT_SCALE;
	 } else {
	 expectedState = LEFT_CLOSED;
	 }

	 display.outputMessage = POLD;

	 break;

	 case RIGHT_OPEN:

	 if (person.direction == -1) {

	 expectedState = RIGHT_CLOSED;
	 } else {
	 expectedState = WEIGHT_SCALE;
	 }

	 display.outputMessage = PORD;

	 break;

	 case LEFT_CLOSED:

	 expectedState = GUARD_LEFT_LOCK;
	 display.outputMessage = LDC;

	 break;

	 case RIGHT_CLOSED:

	 expectedState = GUARD_RIGHT_LOCK;
	 display.outputMessage = RDC;

	 break;

	 case GUARD_LEFT_UNLOCK:

	 expectedState = LEFT_OPEN;
	 display.outputMessage = LDUG;

	 break;

	 case GUARD_RIGHT_UNLOCK:

	 expectedState = RIGHT_OPEN;
	 display.outputMessage = RDUG;

	 break;

	 case GUARD_LEFT_LOCK:

	 //per entered from left, exited to the right
	 //set direction to 0 as no one is there anymore

	 if (person.direction == -1) {

	 expectedState = PERSON_EXIT;
	 } else {
	 expectedState = GUARD_RIGHT_UNLOCK;
	 }

	 display.outputMessage = LDLG;

	 break;

	 case GUARD_RIGHT_LOCK:

	 //per entered from right, exited to the left
	 //set direction to 0 as no one is there anymore

	 if (person.direction == -1) {

	 expectedState = PERSON_EXIT;
	 } else {
	 expectedState = GUARD_LEFT_UNLOCK;
	 }

	 display.outputMessage = RDLG;

	 break;

	 case PERSON_EXIT:

	 person.direction = 0;
	 display.outputMessage = EXITING;

	 break;

	 }

	 if (MsgSend(display_coid, &display, sizeof(display_t), 0, 0) == -1L) {

	 fprintf(stderr, "MsgSend had an error\n");
	 exit(EXIT_FAILURE);

	 }

	 if (display.outputMessage == EXITING) {

	 printf("Exiting Controller\n");
	 break;
	 }

	 }*/		//end of while loop
	//PHASE II VERSION 2
	while (1) {
		rcvPID = MsgReceive(controllerCID, &person, sizeof(person_t), NULL);
		states = (StateFunc)(*states)(person);
		MsgReply(rcvPID, EOK, NULL, 0);

		if (strcmp(person.msg, inMessage[EXIT]) == 0) {
			//sendDisplay(EXITING, person);

			if (MsgSend(display_coid, &display, sizeof(display_t), 0, 0)
					== -1L) {

				fprintf(stderr, "MsgSend had an error\n");
				exit(EXIT_FAILURE);

			}

			break;
		}
	}

	//Phase 3
	ChannelDestroy(controllerCID);
	return EXIT_SUCCESS;
}
