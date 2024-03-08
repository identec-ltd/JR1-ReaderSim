/* This file contains the routines to open, write to, read from and close the serial port. */

#include <windows.h>
#include <stdio.h>
#include "serial.h"

HANDLE hComm;

int ptr = 0;
byte idleState[2] = { 0xFF, 0xFF };
BOOL success = 0;

struct pkt {
	int len;
	int cmd;
	int data[8];
};

int serial_port_open(char* virtualPort)
{
	DCB dcbSerialParams;					// Initializing DCB structure
	BOOL Write_Status;
	COMMTIMEOUTS timeouts = { 0 };

	hComm = CreateFileA(virtualPort,
		GENERIC_READ | GENERIC_WRITE,
		0,    // must be opened with exclusive-access
		NULL, // no security attributes
			OPEN_EXISTING, // must use OPEN_EXISTING
			0,    // not overlapped I/O
			NULL  // hTemplate must be NULL for comm devices
			);

			if (hComm == INVALID_HANDLE_VALUE) {
				if (GetLastError() == ERROR_FILE_NOT_FOUND) {
					puts("cannot open port!");
					return 1;
				}
				puts("invalid handle value!");
				return 1;
			}
			else printf("success\n\rConfiguring port...");

			dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
			Write_Status = GetCommState(hComm, &dcbSerialParams);     //retreives  the current settings
			if (Write_Status == FALSE) {
				printf("Error in GetCommState()");
				CloseHandle(hComm);
				return 1;
			}

			dcbSerialParams.BaudRate = CBR_19200;      // Setting BaudRate = 19200
			dcbSerialParams.ByteSize = 8;             // Setting ByteSize = 8
			dcbSerialParams.StopBits = ONESTOPBIT;    // Setting StopBits = 1
			dcbSerialParams.Parity = NOPARITY;      // Setting Parity = None

			Write_Status = SetCommState(hComm, &dcbSerialParams);  //Configuring the port according to settings in DCB

			if (Write_Status == FALSE) {
				printf("Error in Setting DCB Structure");
				CloseHandle(hComm);
				return 1;
			}

			// Set COM port timeout settings
			timeouts.ReadIntervalTimeout = 50;
			timeouts.ReadTotalTimeoutConstant = 50;
			timeouts.ReadTotalTimeoutMultiplier = 10;
			timeouts.WriteTotalTimeoutConstant = 50;
			timeouts.WriteTotalTimeoutMultiplier = 10;
			if (SetCommTimeouts(hComm, &timeouts) == 0) {
				printf("Error setting timeouts\n");
				CloseHandle(hComm);
				return 1;
			}
			else printf("success\n\r");

			return 0;
}

// To do: prevent buffer overrun
int serial_port_read(char* rx_buffer)
{
	BOOL  Read_Status;										// Status of the various operations
	DWORD dwEventMask = 0;
	unsigned char SerialBuffer[BUFFER_LENGTH + 1] = { 0 };  // Buffer Containing Rxed Data
	int NumBytesRead = 0;									// Bytes read by ReadFile()
	int i = 0, j = 0;
	static BOOL msgStarted = FALSE;
	static BOOL dataReady = FALSE;
	struct pkt rxData = { 0 };
	char cmdBuffer[16];

	// Configure Windows to Monitor the serial device for Character Reception
	Read_Status = SetCommMask(hComm, EV_RXCHAR);
	if (Read_Status == FALSE) {
		printf("Error in Setting CommMask\n\r");
		return -1;
	}

	Read_Status = WaitCommEvent(hComm, &dwEventMask, NULL);		//Wait for the character to be received

	if (Read_Status == FALSE) {
		printf("Error in Setting WaitCommEvent()");
	}
	else {		//If  WaitCommEvent()==True Read the RXed data using ReadFile();
		success = ReadFile(hComm, SerialBuffer, BUFFER_LENGTH, &NumBytesRead, NULL);
	}

	if (NumBytesRead > 0) {
		// Copy command data packet into serial buffer
		for (i = 0; i < NumBytesRead; i++) {
			if (!msgStarted) {
				if (SerialBuffer[i] == 0xA5) {
					ptr = 0;
					msgStarted = TRUE;
				}
			}
			if (msgStarted) {
//				*(rx_buffer + ptr) = SerialBuffer[i] & 0xFF;
				cmdBuffer[ptr++] = SerialBuffer[i];
				if ((ptr > 1) && (ptr == cmdBuffer[1])) { //(int)rx_buffer[1])) {
					dataReady = TRUE;
					break;
				}
			}
		}

		if (dataReady) {
//			if ((*rx_buffer == 0xA5) && (*(rx_buffer + 3) == 0x5A)) {		// valid packet
			if((cmdBuffer[0] == (char)0xA5) && (cmdBuffer[3] == (char)0x5A)) {
				rxData.len = cmdBuffer[1]; //*(rx_buffer + 1);
				rxData.cmd = cmdBuffer[2];	//*(rx_buffer + 2);
			}
//			for (i = 0; i < strlen(rx_buffer); i++) {
//				printf(" %hhx", rx_buffer[i] & 0xFF);
//			}
			dataReady = FALSE;
			msgStarted = FALSE;
			ptr = 0;

			return rxData.cmd;
		}
	}
	
	return 0;
}

int write_serial(char* buf, int length)
{
	int dNoOfBytesWritten = 0;

//	printf("PD: ");
//	for (int i = 0; i < length; i++) {
//		printf("%02X ", *(buf+i) & 0xFF);
//	}
	WriteFile(hComm, idleState, 1, &dNoOfBytesWritten, NULL);			// Send idle byte
	if (!WriteFile(hComm, buf, length, &dNoOfBytesWritten, NULL)) {		// Send packet
		printf("Error writing to serial port\r\n");
		return -1;
	}
//	else {
//		printf("(%d bytes)\n\r", dNoOfBytesWritten);
//	}

	return 0;
}