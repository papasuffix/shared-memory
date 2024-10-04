#include "server.hpp"
#include <iostream>

HANDLE server::shared_memory = NULL;
HANDLE server::interface_event = NULL;
HANDLE server::module_event = NULL;
HANDLE server::mutex = NULL;

server::command_t* server::command = nullptr;

auto server::start( ) -> bool
{
    if ( !setup( ) )
    {
        return false;
    }

    while ( true )
    {
        WaitForSingleObject( interface_event, INFINITE );
        //std::cout << "Interface event triggered" << std::endl;

        if ( tick( ) == false )
        {
            break;
        }
    }

    return true;
}

auto server::send( const char* data, operations operation ) -> bool
{
    DWORD wait_result = WaitForSingleObject( mutex, INFINITE );
    if ( wait_result != WAIT_OBJECT_0 )
    {
        std::cerr << "Failed to get mutex" << std::endl;
        return false;
    }

    if ( command == nullptr )
    {
        ReleaseMutex( mutex );
        std::cerr << "Command pointer is NULL" << std::endl;
        return false;
    }

    command->operation = operation;
    strcpy_s( command->data, data );
    command->process_id = GetCurrentProcessId( );

    if ( !SetEvent( module_event ) )
    {
        ReleaseMutex( mutex );
        std::cerr << "Failed to set module event" << std::endl;
        return false;
    }

    ReleaseMutex( mutex );

    return true;
}

auto server::tick( ) -> bool
{
    bool status = true;

    DWORD wait_result = WaitForSingleObject( mutex, INFINITE );
    if ( wait_result != WAIT_OBJECT_0 )
    {
        std::cerr << "Failed to get mutex" << std::endl;
        return false;
    }

    if ( command == nullptr )
    {
        ReleaseMutex( mutex );
        std::cerr << "Command pointer is NULL" << std::endl;
        return false;
    }

    if ( command->operation != operations::COMPLETE )
    {
        switch ( command->operation )
        {
        case operations::PRINT:
            std::cout << command->data << std::endl;
            break;
        }

        command->operation = operations::COMPLETE;
    }

    ReleaseMutex( mutex );

    return status;
}

auto server::setup( ) -> bool
{
    const wchar_t* shared_memory_name = L"Global\\RainMemory";
    const wchar_t* interface_event_name = L"Global\\InterfaceEvent";
    const wchar_t* module_event_name = L"Global\\ModuleEvent";
    const wchar_t* mutex_name = L"Global\\RainMutex";

    shared_memory = OpenFileMappingW( FILE_MAP_ALL_ACCESS, FALSE, shared_memory_name );
    if ( shared_memory == NULL )
    {
        shared_memory = CreateFileMappingW( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof( command_t ), shared_memory_name );
        if ( shared_memory == NULL )
        {
            DWORD error = GetLastError( );
            std::cerr << "CreateFileMappingW failed with error code: " << error << std::endl;
            return false;
        }
    }

    command = ( command_t* )MapViewOfFile( shared_memory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof( command_t ) );
    if ( command == nullptr )
    {
        DWORD error = GetLastError( );
        std::cerr << "MapViewOfFile failed with error code: " << error << std::endl;
        CloseHandle( shared_memory );
        return false;
    }

    mutex = OpenMutexW( MUTEX_ALL_ACCESS, FALSE, mutex_name );
    if ( mutex == NULL )
    {
        mutex = CreateMutexW( NULL, FALSE, mutex_name );
        if ( mutex == NULL )
        {
            UnmapViewOfFile( command );
            CloseHandle( shared_memory );
            return false;
        }
    }

    interface_event = OpenEventW( EVENT_ALL_ACCESS, FALSE, interface_event_name );
    if ( interface_event == NULL )
    {
        interface_event = CreateEventW( NULL, FALSE, FALSE, interface_event_name );
    }

    module_event = OpenEventW( EVENT_ALL_ACCESS, FALSE, module_event_name );
    if ( module_event == NULL )
    {
        module_event = CreateEventW( NULL, FALSE, FALSE, module_event_name );
    }

    if ( interface_event == NULL || module_event == NULL )
    {
        UnmapViewOfFile( command );
        CloseHandle( shared_memory );
        return false;
    }

    return true;
}

auto server::terminate( ) -> void
{
    if ( command )
    {
        UnmapViewOfFile( command );
        command = nullptr;
    }

    if ( shared_memory )
    {
        CloseHandle( shared_memory );
        shared_memory = NULL;
    }

    if ( interface_event )
    {
        CloseHandle( interface_event );
        interface_event = NULL;
    }

    if ( module_event )
    {
        CloseHandle( module_event );
        module_event = NULL;
    }

    if ( mutex )
    {
        CloseHandle( mutex );
        mutex = NULL;
    }
}