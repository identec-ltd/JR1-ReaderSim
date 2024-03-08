#include <windows.h>
#include <stdio.h>
#include "serial.h"
#include "msgHandler.h"

char* pcCommPort = "COM2";
int status = 0;
char cmd_queue[16];
//char send_buffer[64];

char initCmd[] = { "#init" };
char readCmd[] = { "#read" };
char eraseCmd[] = { "#eras" };
char uninitCmd[] = {"#unin"};

//uint8_t msgData[16] = { 0 };

int process_command_queue(void);
void appInit(void);

void main()
{
	char c;
	boolean tagInit = FALSE;
	
	appInit();

	while (1) {
		printf("Press i to initialise programmer: ");

		// Wait for 'i' to be pressed
		do {
			c = getch(stdin);
		} while ((c != 'i') && (c != 'I'));
		printf("\n\rInitialising programmer...");
		
		// Send init command & read response
		write_serial(initCmd, strlen(initCmd));
		status = serial_port_read(cmd_queue);

		if (status == 0) {
			printf("failed\n\r");
			break;
		}
		printf("done\n\r");
		printf("Response: %02X\n\r", status);
		tagInit = TRUE;

		while (1) {
			printf("\n\rProgrammer initialised\n\rPress r to read tag config\n\r");
			printf("Press e to erase tag\n\rPress u to uninitialise programmer\n\r");

			// Wait for valid command
			do {
				c = getch(stdin);
			} while ((c != 'r') && (c != 'R') && (c != 'e') && (c != 'E') && (c != 'u') && (c != 'U'));

			// Handle read command
			if ((c == 'r') || (c == 'R')) {
				printf("Reading tag...");
				write_serial(readCmd, strlen(readCmd));
				status = serial_port_read(cmd_queue);
				if (status) {
					printf("%02X\n\r", status);
				}
			}
			// Handle erase command
			else if ((c == 'e') || (c == 'E')) {
				printf("Erasing tag config...");
				write_serial(eraseCmd, strlen(eraseCmd));
				status = serial_port_read(cmd_queue);
				if (status) {
					printf("done (%02X)\n\r", status);
				}
			}
			// Handle uninitialise command
			else if ((c == 'u') || (c == 'U')) {
				printf("Uninitialising programmer...");
				write_serial(uninitCmd, strlen(uninitCmd));
				status = serial_port_read(cmd_queue);
				if (status) {
					printf("done (%02X)\n\r", status);
					tagInit = FALSE;
				}
			}
			if (status == 0) {
				printf("Error!\n\r");
				break;
			}
			if (!tagInit) break;
		}
	}
	printf("End of program\n\r");
	while (1);
}


void appInit()
{
	printf("TagProgSim\n\r");

	printf("**********\n\rOpening serial port...");
	status = serial_port_open(pcCommPort);
}