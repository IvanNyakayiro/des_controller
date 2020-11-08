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
		perror("\nFailed to create the channel.  Exiting");
		exit(EXIT_FAILURE);
	}
	/*Connecting to channel created by Des_display*/
	display_coid = ConnectAttach(ND_LOCAL_NODE, displayPID, 1,
	_NTO_SIDE_CHANNEL, 0);
	if (display_coid == -1) {
		printf("\nCannot Connect to Display. Exiting");
		exit(EXIT_FAILURE);
	}
	printf("\nThe controller is running as process_id %d", getpid());

	//setAccepting Person flag to 1;
	acceptingPersonFlag = 1;

	//Phase 2
	/*Server running forever */
	while (1) {

		//get the message from des_input and print
		/* Receive Person object from inputs*/
		rcvPID = MsgReceive(controllerCID, &person, sizeof(person), NULL);

		// validate
		if (rcvPID == -1) {
			fprintf(stderr,"\nError receiving message from Inputs. Exiting.");
		}

		rpyPID = MsgReply(rcvPID, EOK, &msgRply, sizeof(msgRply));

		if (rpyPID == -1) {
			fprintf(stderr,"\nError replying to Inputs.Exting");
			exit(EXIT_FAILURE);
		}

		//Only for when a person hasnt entered yet, handle expected states
		if (person.direction == 0) {
			if (person.state == LEFT_SCAN || person.state == RIGHT_SCAN) {
				expectedState = person.state;
			} else {
				expectedState = -1;
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

	}		//end of while loop

	//Phase 3
	ChannelDestroy(controllerCID);
	return EXIT_SUCCESS;
}
