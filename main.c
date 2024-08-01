#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "serial.h"
#include "msgHandler.h"

char* pcCommPort = "COM8";
int status = 0;
char txBuffer[BUFFER_LENGTH] = { 0 };
char rxBuffer[BUFFER_LENGTH] = { 0 };
BOOL rxStatus = FALSE;
uint8_t txIndex = 0;
uint8_t rxIndex = 0;

uint16_t hid = 0x1234;
uint8_t ledData[3] = { 0 };
uint8_t buzzerData = 1;

//configFile cf;

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
void addChecksum(void);
void handleError(void);
static uint8_t getLEDValue(void);
uint8_t hex2int(char ch);

//void displayConfig(uint8_t *p);
//void displayConfig2(void);

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
	uint8_t checksum = 0;
	time_t currentTime;
	char timeStr[32] = { 0 };

	appInit();											// Open serial port
	
	while (1)
	{
		c = getCommand();								// User input
		txIndex = 0;
		rxIndex = 1;

		switch (c)
		{
		case 'r':		// READ CONFIG
			printf("\n\rReading aerial config...");

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x04);
			addCharToCmd(CMD_READ_CONFIG);
			addChecksum();

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);		// Consider using ReadFile instead to avoid hanging: ReadFile(hComm, &buf, nBytes, &bytesRead, 0)
			} while (rxStatus == FALSE);

			if (rxBuffer[0] == ADDR_AERIAL_1 | 0x80)
			{
				if (rxBuffer[2] == CMD_READ_CONFIG)
				{
					printf("success\n\rConfig: ");
					for (i = 0; i < 32; i++) {
						printf("%02X ", rxBuffer[i + 3]);
					}
					printf("\n\r");
				}
				else if (rxBuffer[2] == CMD_READ_CONFIG | 0x80)
				{
					handleError();
				}
			}
			break;

		case 'x':		// START SCANNING
			printf("\n\rStarting scan...");

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x04);
			addCharToCmd(CMD_START_SCAN);
			addChecksum();

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
					handleError();
				}
			}
			break;

		case 'y':		// STOP_SCANNING
			printf("\n\rStopping scan...");

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x04);
			addCharToCmd(CMD_STOP_SCAN);
			addChecksum();

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
					handleError();
				}
			}
			break;

		case 'g':
			printf("\n\rGetting report...");

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x04);
			addCharToCmd(CMD_GET_REPORT);
			addChecksum();

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
							printf("%u: HID %02hhX%02hhX, PID %02hhX%02hhX%02hhX%02hhX, ", i+1, rxBuffer[5 + (8 * i)],
								rxBuffer[6 + (8 * i)], rxBuffer[7 + (8 * i)], rxBuffer[8 + (8 * i)],
								rxBuffer[9 + (8 * i)], rxBuffer[10 + (8 * i)]);
							printf("RSSI %i, Batt %u\n\r", rxBuffer[11 + (8*i)], rxBuffer[12 + (8*i)]);
						}
					}
				}
				else if (rxBuffer[2] == CMD_GET_REPORT | 0x80)
				{
					handleError();
				}
			}
			break;

		case 'p':		// SET SCAN PARAMETERS
			printf("\n\rWriting scan parameters...");

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x09);
			addCharToCmd(CMD_SET_PARAMS);
			addCharToCmd(0);
			addCharToCmd(1);
			addCharToCmd(244);
			addCharToCmd(1);
			addCharToCmd(244);
			addChecksum();

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if (rxBuffer[0] == ADDR_AERIAL_1 | 0x80)
			{
				if (rxBuffer[2] == CMD_SET_PARAMS)
				{
					printf("success\n\r");
				}
				else if (rxBuffer[2] == CMD_SET_PARAMS | 0x80)
				{
					handleError();
				}
			}
			break;

		case 'q':		// READ SCAN PARAMETERS
			printf("\n\rReading scan parameters...");

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x04);
			addCharToCmd(CMD_GET_PARAMS);
			addChecksum();

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if (rxBuffer[0] == ADDR_AERIAL_1 | 0x80)
			{
				if (rxBuffer[2] == CMD_GET_PARAMS)
				{
					uint16_t i = (rxBuffer[4] << 8) + (uint8_t)rxBuffer[5];
					uint16_t w = (rxBuffer[6] << 8) + (uint8_t)rxBuffer[7];
					printf("success\n\rParameters: ");
					printf("Mode %u, ", rxBuffer[3]);
					printf("Interval %lu, Window %lu\n\r", i, w);
				}
				else if (rxBuffer[2] == CMD_GET_PARAMS | 0x80)
				{
					handleError();
				}
			}
			break;

		case 'h':		// ADD HID
			printf("\n\rAdding HID %04X...", hid);

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x06);
			addCharToCmd(CMD_ADD_HID);
			addCharToCmd((hid >> 8) & 0xFF);
			addCharToCmd(hid & 0xFF);
			addChecksum();
			hid += 0x17;

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if (rxBuffer[0] == ADDR_AERIAL_1 | 0x80)
			{
				if (rxBuffer[2] == CMD_ADD_HID)
				{
					printf("success\n\r");
				}
				else if (rxBuffer[2] == CMD_ADD_HID | 0x80)
				{
					handleError();
				}
			}
			break;

		case 'l':		// LED CONTROL
			for (i = 0; i < 3; i++) {
				if (i == 0) printf("Enter RED value: ");
				else if (i == 1) printf("Enter GREEN value: ");
				else if (i == 2) printf("Enter BLUE value: ");
				ledData[i] = getLEDValue();
			}

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x0A);
			addCharToCmd(CMD_LED);
			addHexByteToCmd(ledData[0]);
			addHexByteToCmd(ledData[1]);
			addHexByteToCmd(ledData[2]);
			addChecksum();
			printf("\n\rSending LED data...");

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if (rxBuffer[0] == ADDR_AERIAL_1 | 0x80)
			{
				if (rxBuffer[2] == CMD_LED)
				{
					printf("success\n\r");
				}
				else if (rxBuffer[2] == CMD_LED | 0x80)
				{
					handleError();
				}
			}
			break;

		case 'b':		// BUZZER CONTROL
			printf("\n\rToggling buzzer...");

			addCharToCmd(ADDR_AERIAL_1);
			addCharToCmd(0x05);
			addCharToCmd(CMD_BUZZER);
			addCharToCmd(buzzerData++);
			addChecksum();
			if (buzzerData >= 2) buzzerData = 0;

			write_serial(txBuffer, txIndex);
			do {
				rxStatus = serial_port_read(rxBuffer);
			} while (rxStatus == FALSE);

			if (rxBuffer[0] == ADDR_AERIAL_1 | 0x80)
			{
				if (rxBuffer[2] == CMD_BUZZER)
				{
					printf("success\n\r");
				}
				else if (rxBuffer[2] == CMD_BUZZER | 0x80)
				{
					handleError();
				}
			}
			break;

		// case 't':	// SET TIME & DATE

		default:
			printf("Unknown command\n\r\n\r");
			memset(txBuffer, 0, sizeof(txBuffer));
			txIndex = 0;
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
	static int8_t count = 5;

	if (count-- == 5) {
		printf("\n\rr - read aerial configuration\n\r");	//s - set aerial configuration\n\r");
		printf("x - start scanning\n\ry - stop scanning\n\r");
		printf("g - get scan report\n\r");	//t - set time & date\n\r");
		printf("h - add hid\n\rl - LED control\n\r");
//		printf("b - buzzer control\n\r"); //p - set scan parameters\n\r");
//		printf("q - read scan parameters\n\r");
	}
	if (count == 0) count = 5;

	do {
		i = getch(stdin);
	} while ((i != 'r') && /*(i != 's') && */(i != 'x') && (i != 'y') && (i != 'g')
		/* && (i != 't')*/ && (i != 'h') && (i != 'l')); // && (i != 'b'));	// && (i != 'p')
//		&& (i != 'q'));

	return i;
}

void addChecksum()
{
	uint8_t i = 0;
	uint8_t chksm = 0;

	for (i = 0; i < txIndex; i++)
	{
		chksm ^= txBuffer[i];
	}
	txBuffer[txIndex++] = chksm;
}

void handleError()
{
	printf("error - ");

	switch (rxBuffer[3])	// Read error code
	{
	case ERROR_INVALID_CMD:
		printf("invalid command\n\r");
		break;
	case ERROR_UNKNOWN_CMD:
		printf("command not recognised\n\r");
		break;
	case ERROR_DATA_OUT_OF_RANGE:
		printf("argument out of range\n\r");
		break;
	case ERROR_UNABLE_TO_EXECUTE:
		printf("cannot execute command\n\r");
		break;
	case ERROR_NOT_SCANNING:
		printf("aerial is not currently scanning");
		break;
	case ERROR_ALREADY_SCANNING:
		printf("aerial is already scanning\n\r");
		break;
	case ERROR_NOT_CONFIGURED:
		printf("aerial is not configured\n\r");
		break;
	case ERROR_HID_LIST_FULL:
		printf("HID list is full\n\r");
		break;
	case ERROR_HID_LIST_EMPTY:
		printf("HID list is empty\n\r");
		break;
	case ERROR_BAD_CHECKSUM:
		printf("bad checksum\n\r");
		break;
	default:
		printf("unknown error\n\r");
		break;
	}
	memset(rxBuffer, 0x00, sizeof(rxBuffer));
}

uint8_t getLEDValue()
{
	uint8_t ret = 0;
	char chIn;

	do {
		chIn = getch(stdin);
	} while (((chIn < 0x30) || (chIn > 0x39)) &&
		((chIn < 0x41) || (chIn > 0x46)) &&
		((chIn < 0x61) || (chIn > 0x66)));
	printf("%c", chIn);
	ret += hex2int(chIn);
	ret <<= 4;

	do {
		chIn = getch(stdin);
	} while (((chIn < 0x30) || (chIn > 0x39)) &&
		((chIn < 0x41) || (chIn > 0x46)) &&
		((chIn < 0x61) || (chIn > 0x66)));
	printf("%c ", chIn);
	ret += hex2int(chIn);
	printf("(%.0f%%)\n\r", (float)(ret / 255.0) * 100.0);

	return ret;
}

uint8_t hex2int(char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	return -1;
}

// For sending data

void addHexByteToCmd(uint8_t b)		// Splits a 2-char byte to 2 bytes
{
	addHexChar((b >> 4) & 0xF);
	addHexChar(b & 0xFF);
}

void addHexChar(uint8_t b)			// Encodes a hex byte to buffer
{
	b &= 0x0F;
	b += (b < 10) ? '0' : 'A' - 10;
	txBuffer[txIndex++] = b;
}

void addCharToCmd(char c)			// Adds a char to buffer
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


