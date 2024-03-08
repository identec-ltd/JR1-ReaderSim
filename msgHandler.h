#pragma once
#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include <stdint.h>

struct messageProperties {
	int length;
	int SCBPresent;
	int CRC_OK;
	int SQN_OK;
	int CMAC_OK;
	int cmd;
	int payloadSize;
	int encryption;
	uint8_t data[32];
};




#endif // !MSGHANDLER_H
