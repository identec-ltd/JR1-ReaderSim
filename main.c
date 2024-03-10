#include <windows.h>
#include <stdio.h>
#include "serial.h"
#include "msgHandler.h"

char* pcCommPort = "COM2";
int status = 0;
char rxBuffer[BUFFER_LENGTH];
BOOL rxStatus = FALSE;

configFile cf;

char initCmd[] = { "#init" };
char readCmd[] = { "#read" };
char eraseCmd[] = { "#eras" };
char uninitCmd[] = {"#unin"};

void appInit(void);

void main()
{
	char c;
	uint8_t i = 0;
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
		do {
			rxStatus = serial_port_read(rxBuffer);
		} while (rxStatus == FALSE);

//		if (status == RX_FAIL) {
//			printf("failed\n\r");
//			break;
//		}
		printf("success\n\r");
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
				rxStatus = serial_port_read(rxBuffer);

//				if (status == RX_SUCCESS) {
					printf("success\n\r");

					printf("HID: ");
					for (i = 2; i < 10; i++) {
						printf("%02hhX", rxBuffer[i]);
					}
					printf("\n\rPID: ");
					for (i = 10; i < 18; i++) {
						printf("%02hhX", rxBuffer[i]);
					}
					printf("\n\rFlags:\t\t\t%hhX\n\r", rxBuffer[18]);
					printf("Inactivity timeout:\t%hhX\n\r", rxBuffer[19]);
					printf("Accel threshold:\t%hhX\n\r", rxBuffer[20]);
					printf("Adv interval:\t\t%hhX\n\r", rxBuffer[21]);
					printf("Adv count:\t\t%hhX\n\r", rxBuffer[22]);
					printf("Tx power:\t\t%hhX\n\r", rxBuffer[23]);
					printf("Log mask:\t\t%hhX\n\r", rxBuffer[24]);
					printf("Shock threshold:\t%hhX\n\r", rxBuffer[25]);
					printf("Security byte:\t\t%hhX\n\r", rxBuffer[26]);

//				}
			}
			// Handle erase command
			else if ((c == 'e') || (c == 'E')) {
				printf("Erasing tag config...");
				write_serial(eraseCmd, strlen(eraseCmd));
				do {
					rxStatus = serial_port_read(rxBuffer);
				} while (rxStatus == FALSE);
//				if (status == RX_SUCCESS) {
					printf("success\n\r");
//				}
			}
			// Handle uninitialise command
			else if ((c == 'u') || (c == 'U')) {
				printf("Uninitialising programmer...");
				write_serial(uninitCmd, strlen(uninitCmd));
				do {
					rxStatus = serial_port_read(rxBuffer);
				} while (rxStatus == FALSE);
//				if (status == RX_SUCCESS) {
					printf("done\n\r");
					tagInit = FALSE;
//				}
			}
/*			if (status == RX_FAIL) {
				printf("failed - ");
				for (i = 0; i < 8; i++) {
					printf("%hhx ", rxBuffer[i]);
				}
				printf("\n\r");
			}*/
			memset(rxBuffer, 0, BUFFER_LENGTH);
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