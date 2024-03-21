#pragma once
#ifndef _SERIAL_H_
#define _SERIAL_H_

#define BUFFER_LENGTH 64

#define CMD_INIT			0x49
#define CMD_READ			0x52
#define CMD_PROG			0x50
#define CMD_ERASE			0x45
#define CMD_SET_OTB			0x53
#define CMD_CLR_OTB			0x54
#define CMD_READ_OTB		0x55
#define CMD_READ_FW			0x46
#define CMD_LED				0x4C

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

#define REPLY_ERROR			0x23
#define ERROR_NOT_INIT      0x00
#define ERROR_UNKNOWN_CMD   0x01
#define ERROR_INVALID_CMD   0x02
#define ERROR_NO_TAG_FOUND  0x03
#define ERROR_OUT_OF_RANGE  0x04
#define ERROR_PROG_DATA     0x05
#define ERROR_OTB_IS_SET    0x06
#define ERROR_CLEAR_OTB     0x07
#define ERROR_SET_OTB       0x08
#define ERROR_ERASE_DATA    0x09
#define NO_ERROR            0xFF
#define RAND_NUM_1			0x4C
#define RAND_NUM_2			0x0A

#define RX_FAIL				0
#define RX_INCOMPLETE		1
#define RX_SUCCESS			2

typedef struct {
	int hid[2];
	int pid[4];
	int flags[4];
	int inactivity_timeout;
	int accel_threshold;
	int adv_interval;
	int adv_count;
	int tx_power;
	int log_mask[2];
	int shock_threshold;
	int reserved[5];
	int security_byte;
} configFile;
// Total size 24 bytes

int serial_port_open(char* virtualPort);
BOOL serial_port_read(char* rx_buffer);
int write_serial(char* buf, int length);


#endif // _SERIAL_H_