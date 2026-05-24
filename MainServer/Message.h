#pragma once

enum OPERATION
{
	ACCEPT,
	RECV,
	SEND
};

struct Message
{
	OPERATION operation;

	int8_t* data;
};