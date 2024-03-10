#pragma once
#ifndef _SERIAL_H_
#define _SERIAL_H_

#define BUFFER_LENGTH 64

#define RX_FAIL			0
#define RX_INCOMPLETE	1
#define RX_SUCCESS		2

typedef struct {
	int hid[8];
	int pid[8];
	int flags;
	int inactivity_timeout;
	int accel_threshold;
	int adv_interval;
	int adv_count;
	int tx_power;
	int log_mask;
	int shock_threshold;
	int security_byte;
} configFile;
// Total size 25 bytes

int serial_port_open(char* virtualPort);
BOOL serial_port_read(char* rx_buffer);
int write_serial(char* buf, int length);


#endif // _SERIAL_H_