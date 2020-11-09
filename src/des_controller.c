#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "./des.h"
/*
 * Serves as client for display
 * Serves as Server for inputs
 *
 */

typedef void *(*StateFunc)();
void sendDisplay(int msg, person_t person);

//global vars for Display
pid_t displayPID;
display_t display;
int display_coid;
int direction; // -1 entering from left, 0 noone is entering, 1 entering from right
int state;

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
void *ACCEPTING_FUNC(person_t person);

void *LEFT_SCAN_FUNC(person_t person) {

	if (direction == LEFT && state == LEFT_SCAN) {

		state = GUARD_LEFT_UNLOCK; //next state
		sendDisplay(ID_SCAN, person);
		return GUARD_LEFT_UNLOCK_FUNC; // next state

	}

	return LEFT_SCAN_FUNC; // next state

}
void *RIGHT_SCAN_FUNC(person_t person) {

	if (direction == RIGHT && state == RIGHT_SCAN) {

		state = GUARD_RIGHT_UNLOCK;
		sendDisplay(ID_SCAN, person);
		return GUARD_RIGHT_UNLOCK_FUNC; // next state

	}
	return RIGHT_SCAN_FUNC; // next state

}
void *WEIGHT_SCALE_FUNC(person_t person) {

	if (direction == RIGHT && state == WEIGHT_SCALE) {

		state = RIGHT_CLOSED;
		sendDisplay(WEIGHED, person);
		return RIGHT_CLOSED_FUNC; // next state

	} else if (direction == LEFT && state == WEIGHT_SCALE) {

		state = LEFT_CLOSED;
		sendDisplay(WEIGHED, person);
		return LEFT_CLOSED_FUNC; // next state

	}
	return WEIGHT_SCALE_FUNC; // next state

}
void *LEFT_OPEN_FUNC(person_t person) {

	if (direction == LEFT && state == LEFT_OPEN) {

		state = WEIGHT_SCALE;
		sendDisplay(POLD, person);
		return WEIGHT_SCALE_FUNC; // next state

	} else if (direction == RIGHT && state == LEFT_OPEN) {

		state = LEFT_CLOSED;
		sendDisplay(POLD, person);
		return LEFT_CLOSED_FUNC; // next state
	}

	return LEFT_OPEN_FUNC; // next state
}
void *RIGHT_OPEN_FUNC(person_t person) {

	if (direction == LEFT && state == RIGHT_OPEN) {

		state = RIGHT_CLOSED;
		sendDisplay(PORD, person);
		return RIGHT_CLOSED_FUNC; // next state

	} else if (direction == RIGHT && state == RIGHT_OPEN) {

		state = WEIGHT_SCALE;
		sendDisplay(PORD, person);
		return WEIGHT_SCALE_FUNC; // next state

	}

	return RIGHT_OPEN_FUNC; // next state
}
void *LEFT_CLOSED_FUNC(person_t person) {

	if (direction == LEFT && state == LEFT_CLOSED) {

		state = GUARD_LEFT_LOCK;
		sendDisplay(LDC, person);
		return GUARD_LEFT_LOCK_FUNC; // next state

	} else if (direction == RIGHT && state == LEFT_CLOSED) {

		state = GUARD_LEFT_LOCK;
		sendDisplay(LDC, person);
		return GUARD_LEFT_LOCK_FUNC; // next state
	}

	return LEFT_CLOSED_FUNC; // next state
}
void *RIGHT_CLOSED_FUNC(person_t person) {

	if (direction == LEFT && state == RIGHT_CLOSED) {

		state = GUARD_RIGHT_LOCK;
		sendDisplay(RDC, person);
		return GUARD_RIGHT_LOCK_FUNC; // next state

	} else if (direction == RIGHT && state == RIGHT_CLOSED) {

		state = GUARD_RIGHT_LOCK;
		sendDisplay(RDC, person);
		return GUARD_RIGHT_LOCK_FUNC; // next state
	}

	return RIGHT_CLOSED_FUNC; // next state
}
void *GUARD_LEFT_UNLOCK_FUNC(person_t person) {

	if (direction == LEFT && state == GUARD_LEFT_UNLOCK) {

		state = LEFT_OPEN;
		sendDisplay(LDUG, person);
		return LEFT_OPEN_FUNC; // next state

	} else if (direction == RIGHT && state == GUARD_LEFT_UNLOCK) {

		state = LEFT_OPEN;
		sendDisplay(LDUG, person);
		return LEFT_OPEN_FUNC; // next state
	}

	return GUARD_LEFT_UNLOCK_FUNC; // next state
}
void *GUARD_RIGHT_UNLOCK_FUNC(person_t person) {

	if (direction == LEFT && state == GUARD_RIGHT_UNLOCK) {

		state = RIGHT_OPEN;
		sendDisplay(RDUG, person);
		return RIGHT_OPEN_FUNC; // next state

	} else if (direction == RIGHT && state == GUARD_RIGHT_UNLOCK) {

		state = RIGHT_OPEN;
		sendDisplay(RDUG, person);
		return RIGHT_OPEN_FUNC; // next state
	}

	return GUARD_RIGHT_UNLOCK_FUNC; // next state
}
void *GUARD_LEFT_LOCK_FUNC(person_t person) {

	if (direction == LEFT && state == GUARD_LEFT_LOCK) {

		state = GUARD_RIGHT_UNLOCK;
		sendDisplay(LDLG, person);
		return GUARD_RIGHT_UNLOCK_FUNC; // next state

	} else if (direction == RIGHT && state == GUARD_LEFT_LOCK) {

		state = ACCEPTING;
		sendDisplay(LDLG, person);
		return ACCEPTING_FUNC(person); // next state

	}

	return GUARD_LEFT_LOCK_FUNC; // next state
}
void *GUARD_RIGHT_LOCK_FUNC(person_t person) {

	if (direction == LEFT && state == GUARD_RIGHT_LOCK) {

		state = ACCEPTING;
		sendDisplay(RDLG, person);
		return ACCEPTING_FUNC(person); // next state

	} else if (direction == RIGHT && state == GUARD_RIGHT_LOCK) {

		state = GUARD_LEFT_UNLOCK;
		sendDisplay(RDLG, person);
		return GUARD_LEFT_UNLOCK_FUNC; // next state

	}

	return GUARD_RIGHT_LOCK_FUNC; // next state
}

void *ACCEPTING_FUNC(person_t person) {

	if (direction == NONE && strcmp(person.msg, "ls") == 0) {

		direction = LEFT;
		state = LEFT_SCAN;
		return LEFT_SCAN_FUNC(person);

	} else if (direction == NONE && strcmp(person.msg, "rs") == 0) {

		direction = RIGHT;
		state = RIGHT_SCAN;
		return RIGHT_SCAN_FUNC(person);

	}

	direction = NONE;
	//state = ACCEPTING;
	sendDisplay(IDLE_MSG, person);

	return ACCEPTING_FUNC;
}

void sendDisplay(int message, person_t person) {

	if (message == ID_SCAN) {
		display.person_id = person.person_id;
	}

	if (message == WEIGHED) {
		display.person_weight = person.weight;
	}

	display.msg = message;

	if (MsgSend(display_coid, &display, sizeof(display_t), 0, 0) == -1L) {

		fprintf(stderr, "MsgSend had an error\n");
		exit(EXIT_FAILURE);

	}

	if (message == EXITING) {
		fprintf(stderr, "Exiting controller\n");
	}

	sleep(3);

}

int main(int argc, char* argv[]) {

	int rcvPID;				// indicates who we should reply to
	int controllerCID;		// channel PID

	person_t person;
	StateFunc states = ACCEPTING_FUNC;

//Phase 1
	if (argc != 2) {

		//print usage message
		fprintf(stderr, "Missing PID to controller\n");
		exit(EXIT_FAILURE);

	}

	displayPID = atoi(argv[1]);

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
	fprintf(stderr, "The controller is running as PID: %d\n", getpid());

	display.msg = IDLE_MSG;
	MsgSend(display_coid, &display, sizeof(display_t), NULL, 0);

	//PHASE II VERSION 2
	while (1) {

		rcvPID = MsgReceive(controllerCID, &person, sizeof(person_t), NULL);
		states = (StateFunc) (*states)(person);
		MsgReply(rcvPID, EOK, &person, sizeof(person_t));

		if (strcmp(person.msg, inMessage[EXIT]) == 0) {
			sendDisplay(EXITING, person);
			break;
		}
	}

	//Phase 3
	ChannelDestroy(controllerCID);
	return EXIT_SUCCESS;
}
