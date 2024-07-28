/* This file contains the routines to open, write to, read from and close the serial port. */

#include <windows.h>
#include <stdio.h>
#include "serial.h"

HANDLE hComm;

//int ptr = 0;
byte idleState[2] = { 0xFF, 0xFF };
BOOL success = 0;

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

			dcbSerialParams.BaudRate = CBR_115200;      // Setting BaudRate = 19200
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
			timeouts.ReadIntervalTimeout = 150;					// max time(ms) allowed between bytes before returning
			timeouts.ReadTotalTimeoutConstant = 500;			// added to calc below
			timeouts.ReadTotalTimeoutMultiplier = 0;			// used to calculate total read timeout(ms) - x no bytes requested
			timeouts.WriteTotalTimeoutConstant = 50;			// added to calc below
			timeouts.WriteTotalTimeoutMultiplier = 10;			// used to calculate total write timeout(ms) - x no bytes to be written
			if (SetCommTimeouts(hComm, &timeouts) == 0) {
				printf("Error setting timeouts\n");
				CloseHandle(hComm);
				return 1;
			}
			else printf("success\n\r");

			return 0;
}

// To do: prevent buffer overrun
BOOL serial_port_read(char* rx_buffer)
{
	BOOL  Read_Status;										// Status of the various operations
	DWORD dwEventMask = 0;
	unsigned char SerialBuffer[BUFFER_LENGTH + 1] = { 0 };  // Buffer Containing Rxed Data
	int NumBytesRead = 0;									// Bytes read by ReadFile()
	int i = 0;
	static BOOL msgStarted = FALSE;
	static BOOL dataReady = FALSE;
	static int ptr = 0;
	int success = 0;

	// Configure Windows to monitor the serial device for character reception
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
		if (ptr >= BUFFER_LENGTH) {
			ptr = 0;
			msgStarted = FALSE;
		}
		// Copy command data packet into serial buffer
		for (i = 0; i < NumBytesRead; i++) {
			if (!msgStarted) {
				if (SerialBuffer[i] != 0xFF) {
					dataReady = FALSE;
					ptr = 0;
					msgStarted = TRUE;
				}
			}
			if (msgStarted) {
				*(rx_buffer+ptr) = SerialBuffer[i];		// Copy Rx data to rx_buffer
				if ((ptr > 2) && (ptr == rx_buffer[1] - 1))
				{
					dataReady = TRUE;
					msgStarted = FALSE;
					break;
				}
				ptr++;
			}
		}
	}
	return dataReady;
}

int write_serial(char* buf, int length)
{
	int dNoOfBytesWritten = 0;

	WriteFile(hComm, idleState, 1, &dNoOfBytesWritten, NULL);			// Send idle byte
	if (!WriteFile(hComm, buf, length, &dNoOfBytesWritten, NULL)) {		// Send packet
		printf("Error writing to serial port\r\n");
		return -1;
	}

	return 0;
}