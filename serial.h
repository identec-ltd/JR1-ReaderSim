#pragma once
#ifndef _SERIAL_H_
#define _SERIAL_H_

#define BUFFER_LENGTH 64

int serial_port_open(char* virtualPort);
int serial_port_read(char* rx_buffer);
int write_serial(char* buf, int length);




#endif // _SERIAL_H_