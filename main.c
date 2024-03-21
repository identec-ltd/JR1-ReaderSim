#include <windows.h>
#include <stdio.h>
#include "serial.h"
#include "msgHandler.h"

char* pcCommPort = "COM2";
int status = 0;
char txBuffer[BUFFER_LENGTH] = { 0 };
char rxBuffer[BUFFER_LENGTH] = { 0 };
BOOL rxStatus = FALSE;

configFile cf;
uint8_t prog[] = {	0x23, 0x32,
					0x44, 0x55, 0x44, 0x55,
					0x01, 0x00, 0x00, 0x12,
					0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00,
					0x01, 0x01, 0x00, 0x00,
					0x00, 0xA5 };

void appInit(void);
char getCommand(void);
void handleError(void);
void displayConfig(uint8_t *file);

void main()
{
	char c, chIn;
	uint8_t i = 0;
	boolean tagInit = FALSE;
	uint16_t check = 0;

	appInit();

	while (1) {
		c = getCommand();

		switch (c)
		{
		case 'i':		// INITIALISE
			printf("\n\rInitialising programmer...");

			txBuffer[0] = CMD_INIT;
			txBuffer[1] = RAND_NUM_1;
			txBuffer[2] = RAND_NUM_2;
			txBuffer[3] = RAND_NUM_1 ^ 0x55;
			txBuffer[4] = RAND_NUM_2 ^ 0xAA;
			txBuffer[5] = CMD_TERMINATE;

			write_serial(txBuffer, 6);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] = REPLY_INIT) && (rxBuffer[1] == 0x31)) {
				printf("success\n\r");
			}
			else handleError();

			break;

		case 'j':		// UNINITIALISE
			printf("\n\rUninitialising programmer...");

			txBuffer[0] = CMD_INIT;
			memset(txBuffer + 1, 0x00, 4);
			txBuffer[5] = CMD_TERMINATE;

			write_serial(txBuffer, 6);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] = REPLY_INIT) && (rxBuffer[1] == 0x30)) {
				printf("success\n\r");
			}
			else handleError();

			break;

		case 'r':		// READ TAG
			printf("\n\rReading tag configuration...");

			txBuffer[0] = CMD_READ;
			txBuffer[1] = CMD_TERMINATE;

			write_serial(txBuffer, 2);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_READ) && (rxBuffer[25] == CMD_PROMPT) && (rxBuffer[26] == CMD_TERMINATE)) {
				printf("success\n\r\n\r");
				displayConfig(rxBuffer);
			}
			else handleError();

			break;

		case 'p':		// PROGRAM TAG
			printf("\n\rConfiguration programming...");

			txBuffer[0] = CMD_PROG;
			memcpy(txBuffer + 1, prog, 24);
			txBuffer[25] = CMD_TERMINATE;

			write_serial(txBuffer, 26);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_PROG) && (rxBuffer[25] == CMD_PROMPT) && (rxBuffer[26] == CMD_TERMINATE)) {
				printf("success\n\r");
				displayConfig(rxBuffer);
			}
			else handleError();
			break;

		case 'e':		// ERASE TAG
			printf("\n\rErasing tag configuration...");

			txBuffer[0] = CMD_ERASE;
			memcpy(txBuffer + 1, cf.hid, 2);
			memcpy(txBuffer + 3, cf.pid, 4);
			txBuffer[7] = CMD_TERMINATE;

			write_serial(txBuffer, 8);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_ERASE) && (rxBuffer[25] == CMD_PROMPT) && (rxBuffer[26] == CMD_TERMINATE)) {
				printf("success\n\r");
				displayConfig(rxBuffer);
			}
			else handleError();
			break;

		case 's':		// SET OTB
			printf("\n\rSetting OTB...");

			txBuffer[0] = CMD_SET_OTB;
			memcpy(txBuffer + 1, cf.hid, 2);
			memcpy(txBuffer + 3, cf.pid, 4);
			txBuffer[7] = CMD_TERMINATE;

			write_serial(txBuffer, 8);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_SET_OTB) && (rxBuffer[1] == 0x31) &&
				(rxBuffer[2] == CMD_PROMPT) && (rxBuffer[3] == CMD_TERMINATE)) {
				printf("success\n\r");
			}
			else handleError();
			break;

		case 't':		// CLEAR OTB
			printf("\n\rClearing OTB...");

			txBuffer[0] = CMD_CLR_OTB;
			memcpy(txBuffer + 1, cf.hid, 2);
			memcpy(txBuffer + 3, cf.pid, 4);
			txBuffer[7] = CMD_TERMINATE;

			write_serial(txBuffer, 8);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_CLR_OTB) && (rxBuffer[1] == 0x31) &&
				(rxBuffer[2] == CMD_PROMPT) && (rxBuffer[3] == CMD_TERMINATE)) {
				printf("success\n\r");
			}
			else handleError();
			break;

		case 'u':		// READ OTB
			printf("\n\rReading OTB...");

			txBuffer[0] = CMD_READ_OTB;
			memcpy(txBuffer + 1, cf.hid, 2);
			memcpy(txBuffer + 3, cf.pid, 4);
			txBuffer[7] = CMD_TERMINATE;

			write_serial(txBuffer, 8);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_READ_OTB) && (rxBuffer[2] == CMD_PROMPT) && (rxBuffer[3] == CMD_TERMINATE)) {
				if (rxBuffer[1] == 0x00) printf("currently CLEARED\n\r");
				else if (rxBuffer[1] == 0x01) printf("currently SET\n\r");
				else printf("status unknown\n\r");
			}
			else handleError();
			break;

		case 'f':		// READ FW VERSION
			printf("\n\rGetting firmware version...");

			txBuffer[0] = CMD_READ_FW;
			txBuffer[1] = CMD_TERMINATE;

			write_serial(txBuffer, 2);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_READ_FW) && (rxBuffer[11] == CMD_PROMPT) && (rxBuffer[12] == CMD_TERMINATE)) {
				printf("success\n\rFirmware: ");
				for (i = 1; i < 11; i++) {
					printf("%02X ", rxBuffer[i]);
				}
				printf("\n\r");
			}
			else handleError();
			break;

		case 'l':		// SET LEDS
			printf("\n\rLED control\n\rEnter LED 3 state: ");

			txBuffer[0] = CMD_LED;
			do {
				chIn = getch(stdin);
			} while ((chIn != '0') && (chIn != '1'));
			if (chIn == '0') txBuffer[1] = 0x30;
			else txBuffer[1] = 0x31;

			printf("%c\n\rEnter LED4 state: ", chIn);
			do {
				chIn = getch(stdin);
			} while ((chIn != '0') && (chIn != '1'));
			if (chIn == '0') txBuffer[2] = 0x30;
			else txBuffer[2] = 0x31;
			printf("%c\n\r", chIn);

			txBuffer[3] = CMD_TERMINATE;

			write_serial(txBuffer, 4);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == CMD_PROMPT) && (rxBuffer[1] == CMD_TERMINATE)) {
				printf("Done\n\r");
			}
			else handleError();

			break;


		default:
			printf("Unknown command\n\r\n\r");
			break;
		}
	}


	/*			if (status == RX_FAIL) {
					printf("failed - ");
					for (i = 0; i < 8; i++) {
						printf("%hhx ", rxBuffer[i]);
					}
					printf("\n\r");
				}
				memset(rxBuffer, 0, BUFFER_LENGTH);
				if (!tagInit) break;
			}
		}
		printf("End of program\n\r");
		while (1);*/
}


void appInit()
{
	printf("TagProgSim\n\r");

	printf("**********\n\rOpening serial port...");
	status = serial_port_open(pcCommPort);

	if (status == 1)
	{
		printf("\n\r\n\rEnd of program");
		while (1);
	}
}

char getCommand()
{
	char i;

	printf("\n\ri - initialise programmer\n\rj - uninitialise programmer\n\r");
	printf("r - read tag configuration\n\rp - program configuration data\n\r");
	printf("e - erase tag configuration\n\rs - set one-time bit\n\r");
	printf("t - clear one-time bit\n\ru - read one-time bit\n\r");
	printf("f - read firmware version\n\rl - LED control\n\r");

	do {
		i = getch(stdin);
	} while ((i != 'i') && (i != 'j') && (i != 'r') && (i != 'p') && (i != 'e')
		&& (i != 's') && (i != 't') && (i != 'u') && (i != 'f') && (i != 'l'));

	return i;
}

void handleError()
{
	if ((rxBuffer[0] == 0x23) && (rxBuffer[2] == CMD_PROMPT) && (rxBuffer[3] == CMD_TERMINATE))
	{
		printf("error - ");
		switch (rxBuffer[1])
		{
		case ERROR_NOT_INIT:
			printf("programmer not initialised\n\r");
			break;
		case ERROR_UNKNOWN_CMD:
			printf("command not recognised\n\r");
			break;
		case ERROR_INVALID_CMD:
			printf("invalid command\n\r");
			break;
		case ERROR_NO_TAG_FOUND:
			printf("no tag found\n\r");
			break;
		case ERROR_OUT_OF_RANGE:
			printf("value out of range");
			break;
		case ERROR_PROG_DATA:
			printf("unable to program\n\r");
			break;
		case ERROR_OTB_IS_SET:
			printf("one-time bit is set\n\r");
			break;
		case ERROR_CLEAR_OTB:
			printf("unable to clear one-time bit\n\r");
			break;
		case ERROR_SET_OTB:
			printf("unable to set one-time bit\n\r");
			break;
		case ERROR_ERASE_DATA:
			printf("unable to erase configuration data\n\r");
			break;
		default:
			printf("unknown error\n\r");
			break;
		}
	}


}

void displayConfig(uint8_t* file)
{
	uint8_t i = 0;

	printf("HID:\t\t\t");
	for (i = 1; i < 3; i++) {
		printf("%02hhX", *(file + i));
	}
	printf("\n\rPID:\t\t\t");
	for (i = 3; i < 7; i++) {
		printf("%02hhX", *(file + i));
	}
	printf("\n\rFlags:\t\t\t");
	for (i = 7; i < 11; i++) {
		printf("%02hhX", *(file + i));
	}
	printf("\n\rInactivity timeout:\t%02hhX\n\r", *(file + 11));
	printf("Accel threshold:\t%02hhX\n\r", *(file + 12));
	printf("Adv interval:\t\t%02hhX\n\r", *(file + 13));
	printf("Adv count:\t\t%02hhX\n\r", *(file + 14));
	printf("Tx power:\t\t%02hhX\n\r", *(file + 15));
	printf("Log mask:\t\t%02hhX%02hhX\n\r", *(file + 16), *(file + 17));
	printf("Shock threshold:\t%02hhX\n\r", *(file + 18));
	printf("Reserved: \t\t");
	for (i = 19; i < 24; i++) {
		printf("%02hhX", *(file + i));
	}
	printf("\n\rSecurity byte:\t\t%02hhX\n\r", *(file + 24));
}