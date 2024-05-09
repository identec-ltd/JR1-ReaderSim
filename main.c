#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "serial.h"
#include "msgHandler.h"

char* pcCommPort = "COM3";
int status = 0;
char txBuffer[BUFFER_LENGTH] = { 0 };
char rxBuffer[BUFFER_LENGTH] = { 0 };
BOOL rxStatus = FALSE;
uint8_t txIndex = 0;
uint8_t rxIndex = 0;

configFile cf;
uint8_t prog[] = {	0x00, 0x00,					// log mask
					0x12, 0x34,					// hid
					0x80, 0x81, 0x82, 0x83,		// pid
					0x00, 0x00, 0x00, 0x00,		// flags
					0x64, 0x10, 0x04, 0x02,		// inac, acc, advi, advc
					0x10, 0x2c, 				// txp, shk
					0x00, 0x00, 0x00, 0x00,		// reserved x13
					0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00,
					0x00,
					0x00 };						// checksum
					
uint8_t rxConfig[32] = { 0 };

typedef struct
{
	unsigned nibble0 : 4;
	unsigned nibble1 : 4;
} stByteNibbles;

typedef union
{
	unsigned char   byte;
	stByteNibbles   Nibbles;
} unByte;

void appInit(void);
char getCommand(void);
void handleError(void);
void displayConfig(uint8_t *p);
void displayConfig2(void);

void addHexByteToCmd(uint8_t b);
void addHexChar(uint8_t b);
void addCharToCmd(char c);

//void processCmdFile(uint8_t* p);
unsigned char cmdBuffer_GetHexChar(void);
unsigned char cmdBuffer_GetByte(void);

void main()
{
	char c, chIn;
	uint8_t i = 0, numTags = 0;
//	uint16_t check = 0;
	uint8_t checksum = 0;
	time_t currentTime;
	char timeStr[32] = { 0 };

	appInit();
	
	while (1)
	{
		c = getCommand();
		txIndex = 0;
		rxIndex = 1;

		switch (c)
		{
		case 'r':		// READ CONFIG
			printf("\n\rReading aerial config...");

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x04);
			addCharToCmd(CMD_READ_CONFIG);
			addCharToCmd(0x5A);			// Checksum

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if (rxBuffer[0] == ADDR_AERIAL_1 | 0x80)
			{
				if (rxBuffer[2] == CMD_READ_CONFIG)
				{
					printf("success\n\r");
				}
				else if (rxBuffer[2] == CMD_READ_CONFIG | 0x80)
				{
					printf("Error\n\r");
				}
			}
			//else handleError();
			else printf("Bad address\n\r");

			break;

		case 'x':		// START SCANNING
			printf("\n\rStarting scan...");

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x04);
			addCharToCmd(CMD_START_SCAN);
			addCharToCmd(0x5A);

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if (rxBuffer[0] == ADDR_AERIAL_1 | 0x80)
			{
				if (rxBuffer[2] == CMD_START_SCAN)
			{
				printf("success\n\r");
			}
				else if (rxBuffer[2] == CMD_START_SCAN | 0x80)
				{
					printf("Error\n\r");
				}
			}
			else printf("Bad address\n\r");

			break;

		case 'y':		// STOP_SCANNING
			printf("\n\rStopping scan...");

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x04);
			addCharToCmd(CMD_STOP_SCAN);
			addCharToCmd(0x5A);

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if (rxBuffer[0] == ADDR_AERIAL_1 | 0x80)
			{
				if (rxBuffer[2] == CMD_STOP_SCAN)
				{
					printf("success\n\r");
				}
				else if (rxBuffer[2] == CMD_STOP_SCAN | 0x80)
				{
					printf("Error\n\r");
				}
			}
			else printf("Bad address\n\r");

			break;

		case 'g':
			printf("\n\rGetting report...");

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x04);
			addCharToCmd(CMD_GET_REPORT);
			addCharToCmd(0x5A);

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if (rxBuffer[0] == ADDR_AERIAL_1 | 0x80)
			{
				if (rxBuffer[2] == CMD_GET_REPORT)
				{
					numTags = (rxBuffer[1] - 6)/8;
					printf("%u tags found\n\r", numTags);

					if (numTags > 0) {
						for (i = 0; i < numTags; i++) {
							printf("%u: HID %02hhX%02hhX, PID %02hhX%02hhX%02hhX%02hhX\n\r", i+1, rxBuffer[5 + (8 * i)],
								rxBuffer[6 + (8 * i)], rxBuffer[7 + (8 * i)], rxBuffer[8 + (8 * i)],
								rxBuffer[9 + (8 * i)], rxBuffer[10 + (8 * i)]);
						}
					}
				}
				else if (rxBuffer[2] == CMD_GET_REPORT | 0x80)
				{
					printf("Error\n\r");
				}
			}
			else printf("Bad address\n\r");

			break;

/*		case 'r':		// READ TAG
			printf("\n\rReading tag configuration...");

			addCharToCmd(CMD_READ);
			addCharToCmd(CMD_TERMINATE);

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_READ) && (rxBuffer[65] == CMD_PROMPT) && (rxBuffer[66] == CMD_TERMINATE)) {
				printf("success\n\r\n\r");
				displayConfig(rxBuffer);
				memcpy(rxConfig, rxBuffer + 1, sizeof(rxConfig));
//				displayConfig2();
			}
			else handleError();

			break;

		case 'p':		// PROGRAM TAG
			printf("\n\rConfiguration programming...");

			addCharToCmd(CMD_PROG);

			checksum = prog[0];								// Calculate & append checksum
			for (i = 0; i < sizeof(prog)-1; i++)
			{
				checksum ^= prog[i];
			}
			prog[31] = checksum;

			for (i = 0; i < sizeof(prog); i++) {			// Send predefined 'prog' config file
				addHexByteToCmd(prog[i]);
			}
			addCharToCmd(CMD_TERMINATE);

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_PROG) && (rxBuffer[65] == CMD_PROMPT) && (rxBuffer[66] == CMD_TERMINATE)) {
				printf("success\n\r");
				displayConfig(rxBuffer);
				for (i = 0; i < sizeof(rxConfig); i++) {
					rxConfig[i] = (rxBuffer[rxIndex] - '0') << 4;
					rxConfig[i] += (rxBuffer[rxIndex + 1] - '0');
					rxIndex += 2;
				}
			}
			else handleError();

			break;

		case 'e':		// ERASE TAG
			printf("\n\rErasing tag configuration...");

			addCharToCmd(CMD_ERASE);
			for(i = 0; i < 6; i++) {
				addHexByteToCmd(0x00);
			}
			addCharToCmd(CMD_TERMINATE);

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_ERASE) && (rxBuffer[65] == CMD_PROMPT) && (rxBuffer[66] == CMD_TERMINATE)) {
				printf("success\n\r");
				displayConfig(rxBuffer);
				memcpy(rxConfig, rxBuffer + 1, sizeof(rxConfig));
			}
			else handleError();

			break;

		case 's':		// SET OTB
			printf("\n\rSetting OTB...");

			addCharToCmd(CMD_SET_OTB);
			for (i = 0; i < 6; i++) {
				addHexByteToCmd(rxConfig[i]);
			}
			addCharToCmd(CMD_TERMINATE);

			write_serial(txBuffer, txIndex);
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

			addCharToCmd(CMD_CLR_OTB);
			for (i = 2; i < 8; i++) {
				addHexByteToCmd(rxConfig[i]);
			}
			addCharToCmd(CMD_TERMINATE);

			write_serial(txBuffer, txIndex);
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

			addCharToCmd(CMD_READ_OTB);
			for (i = 0; i < 6; i++) {
				addHexByteToCmd(rxConfig[i]);
			}
			addCharToCmd(CMD_TERMINATE);

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_READ_OTB) && (rxBuffer[2] == CMD_PROMPT) && (rxBuffer[3] == CMD_TERMINATE)) {
				if (rxBuffer[1] == 0x30) printf("currently CLEARED\n\r");
				else if (rxBuffer[1] == 0x31) printf("currently SET\n\r");
				else printf("status unknown\n\r");
			}
			else handleError();

			break;

		case 'f':		// READ FW VERSION
			printf("\n\rGetting firmware version...");

			addCharToCmd(CMD_READ_FW);
			addCharToCmd(CMD_TERMINATE);

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_READ_FW) && (rxBuffer[11] == CMD_PROMPT) && (rxBuffer[12] == CMD_TERMINATE)) {
				printf("success\n\rFirmware: ");
				for (i = 1; i < 11; i++) {
					printf("%c", rxBuffer[i]);
				}
				printf("\n\r");
			}
			else handleError();

			break;

		case 'l':		// SET LEDS
			printf("\n\rLED control\n\rEnter LED3 state: ");

			addCharToCmd(CMD_LED);
			do {
				chIn = getch(stdin);
			} while ((chIn != '0') && (chIn != '1'));
			if (chIn == '0') addCharToCmd(0x30);
			else addCharToCmd(0x31);

			printf("%c\n\rEnter LED4 state: ", chIn);
			do {
				chIn = getch(stdin);
			} while ((chIn != '0') && (chIn != '1'));
			if (chIn == '0') addCharToCmd(0x30);
			else addCharToCmd(0x31);
			printf("%c\n\r", chIn);

			addCharToCmd(CMD_TERMINATE);

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == CMD_PROMPT) && (rxBuffer[1] == CMD_TERMINATE)) {
				printf("Done\n\r");
			}
			else handleError();

			break;

		case 'x':		// Set time & Date
			printf("Programming time & date...");

			currentTime = time(NULL);
			ctime_s(timeStr, sizeof(timeStr), &currentTime);
			timeStr[strlen(timeStr) - 1] = '\0';

			printf("%s\n\r", timeStr);
			//printf("%lX", currentTime);

			addCharToCmd(CMD_WRITE_TIME);
			addHexByteToCmd((currentTime >> 24) & 0xFF);
			addHexByteToCmd((currentTime >> 16) & 0xFF);
			addHexByteToCmd((currentTime >> 8) & 0xFF);
			addHexByteToCmd((currentTime) & 0xFF);
			addCharToCmd(CMD_TERMINATE);

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_WRITE_TIME) && (rxBuffer[2] == CMD_PROMPT) && (rxBuffer[3] == CMD_TERMINATE))
			{
				if (rxBuffer[2] == 0x31) {
					printf("success\n\r");
				}
			}
			else handleError();

			break;

		case 'y':
			printf("Reading time & date...");

			addCharToCmd(CMD_READ_TIME);
			addCharToCmd(CMD_TERMINATE);

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if ((rxBuffer[0] == REPLY_READ_TIME) && (rxBuffer[9] == CMD_PROMPT) && 
				(rxBuffer[10] == CMD_TERMINATE)) {
				currentTime = 0;
				//rxIndex = 1;

				for (i = 0; i < 4; i++) {
					currentTime += cmdBuffer_GetByte() << ((3 - i) * 8);
				}
				ctime_s(timeStr, sizeof(timeStr), &currentTime);
				timeStr[strlen(timeStr) - 1] = '\0';

				printf("%s\n\r", timeStr);
			}
			else handleError();

			break;*/

		default:
			printf("Unknown command\n\r\n\r");

			break;
		}
	}
}


void appInit()
{
	printf("ReaderSim\n\r");

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

	printf("\n\rr - read aerial configuration\n\rs - set aerial configuration\n\r");
	printf("x - start scanning\n\ry - stop scanning\n\r");
	printf("g - get scan report\n\rt - set time & date\n\r");
	printf("h - add hid\n\rl - LED control\n\r");
	printf("b - buzzer control\n\p - set scan parameters\n\r");
	printf("q - read scan parameters\n\r");//y - read time & date\n\r");
//	printf("f - read firmware version\n\rl - LED control\n\r");

	do {
		i = getch(stdin);
	} while ((i != 'r') && (i != 's') && (i != 'x') && (i != 'y') && (i != 'g')
		&& (i != 't') && (i != 'h') && (i != 'l') && (i != 'b') && (i != 'p')
		&& (i != 'q'));// && (i != 'd') && (i != 'x') && (i != 'y'));

	return i;
}

void handleError()
{
	if ((rxBuffer[0] == 0x23) && (rxBuffer[3] == CMD_PROMPT) && (rxBuffer[4] == CMD_TERMINATE))
	{
		printf("error - ");
		switch (rxBuffer[2])
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
		case 0x41:		//ERROR_NO_TAG_CONN:
			printf("no tag connected\n\r");
			break;
		default:
			printf("unknown error\n\r");
			break;
		}
	}

	memset(rxBuffer, 0x00, sizeof(rxBuffer));
}

void addHexByteToCmd(uint8_t b)
{
	addHexChar((b >> 4) & 0xF);
	addHexChar(b & 0xFF);
}

void addHexChar(uint8_t b)
{
	b &= 0x0F;
	b += (b < 10) ? '0' : 'A' - 10;
	txBuffer[txIndex++] = b;
}

void addCharToCmd(char c)
{
	txBuffer[txIndex++] = c;
}

// For decoding incoming packets...

unsigned char cmdBuffer_GetHexChar() // reads buffer and converts value to a 4 bit number
{
	unsigned char c;

	c = rxBuffer[rxIndex++] - '0';
	c = (c > 9) ? c - 7 : c;
	return c;
}

unsigned char cmdBuffer_GetByte()    // reads 8 bits from usb_buffer
{
	unByte b;

	b.Nibbles.nibble1 = cmdBuffer_GetHexChar();
	b.Nibbles.nibble0 = cmdBuffer_GetHexChar();
	return b.byte;
}


void displayConfig(uint8_t* p)
{
	uint8_t i = 0;
	p += 1;
	
	printf("HID:\t\t\t");
	for (i = 4; i < 8; i++) {
		printf("%c", *(p+i));
	}
	printf("\n\rPID:\t\t\t");
	for (i = 8; i < 16; i++) {
		printf("%c", *(p+i));
	}
	printf("\n\rFlags:\t\t\t");
	for (i = 16; i < 24; i++) {
		printf("%c", *(p+i));
	}
	printf("\n\rInactivity timeout:\t%c%c\n\r", *(p+24), *(p+25));
	printf("Accel threshold:\t%c%c\n\r", *(p+26), *(p+27));
	printf("Adv interval:\t\t%c%c\n\r", *(p+28), *(p+29));
	printf("Adv count:\t\t%c%c\n\r", *(p+30), *(p+31));
	printf("Tx power:\t\t%c%c\n\r", *(p+32), *(p+33));
	printf("Log mask:\t\t");
	for (i = 0; i < 4; i++) {
		printf("%c", *(p+i));
	}
	printf("\n\rShock threshold:\t%c%c\n\r", *(p+34), *(p+35));
	printf("Reserved: \t\t");
	for (i = 36; i < 62; i++) {
		printf("%c", *(p+i));
	}
	printf("\n\rSecurity byte:\t\t%c%c\n\r", *(p+62), *(p+63));
}

void displayConfig2()
{
	printf("HID:\t\t\t");



}