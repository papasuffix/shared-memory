#pragma once

#include <Windows.h>

#include <string>

class server
{
public:
	enum operations
	{
		PRINT,
		COMPLETE
	};

	static auto send( const char* data, operations operation ) -> bool;
	static auto start( ) -> bool;
private:
	struct command_t
	{
		operations operation;
		DWORD process_id;
		char data[256];
	};

	static HANDLE shared_memory;
	static HANDLE interface_event;
	static HANDLE module_event;
	static HANDLE mutex;

	static command_t* command;

	static auto tick( ) -> bool;
	static auto setup( ) -> bool;
	static auto terminate( ) -> void;
};