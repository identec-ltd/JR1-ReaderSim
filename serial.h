#pragma once
#ifndef _SERIAL_H_
#define _SERIAL_H_

#define BUFFER_LENGTH		64

#define ADDR_AERIAL_1		0x01
#define ADDR_AERIAL_2		0x02

#define CMD_READ_CONFIG		0x52	// R
#define CMD_SET_CONFIG		0x53	// S
#define CMD_START_SCAN		0x58	// X
#define CMD_STOP_SCAN		0x59	// Y
#define CMD_GET_REPORT		0x47	// G
#define CMD_SET_TIME_DATE	0x54	// T
#define CMD_LED				0x4C	// L
#define CMD_BUZZER			0x42	// B
#define CMD_SET_PARAMS		0x50	// P
#define CMD_GET_PARAMS		0x51	// Q

#define CMD_PROMPT			0x3E
#define CMD_TERMINATE		0x0D

#define REPLY_INIT			0x69
#define REPLY_READ			0x72
#define REPLY_PROG			0x70
#define REPLY_ERASE			0x65
#define REPLY_SET_OTB		0x73
#define REPLY_CLR_OTB		0x74
#define REPLY_READ_OTB		0x75
#define REPLY_READ_FW		0x66
#define REPLY_CONN			0x63
#define REPLY_DISC			0x64
#define REPLY_WRITE_TIME	0x78
#define REPLY_READ_TIME		0x79

#define REPLY_ERROR			0x23
#define ERROR_NOT_INIT      0x30
#define ERROR_UNKNOWN_CMD   0x31
#define ERROR_INVALID_CMD   0x32
#define ERROR_NO_TAG_FOUND  0x33
#define ERROR_OUT_OF_RANGE  0x34
#define ERROR_PROG_DATA     0x35
#define ERROR_OTB_IS_SET    0x36
#define ERROR_CLEAR_OTB     0x37
#define ERROR_SET_OTB       0x38
#define ERROR_ERASE_DATA    0x39
#define ERROR_NO_TAG_CONN	0x41
#define ERROR_NO_ERROR      0xFF

#define RX_FAIL				0
#define RX_INCOMPLETE		1
#define RX_SUCCESS			2

typedef struct {
	int log_mask[2];
	int hid[2];
	int pid[4];
	int flags[4];
	int inactivity_timeout;
	int accel_threshold;
	int adv_interval;
	int adv_count;
	int tx_power;
	int shock_threshold;
	int reserved[13];
	int security_byte;
} configFile;
// Total size 32 bytes*/

int serial_port_open(char* virtualPort);
BOOL serial_port_read(char* rx_buffer);
int write_serial(char* buf, int length);


#endif // _SERIAL_H_