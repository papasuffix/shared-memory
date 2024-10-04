#include "workspace/communication/server.hpp"

#include <thread>
#include <iostream>

int main( )
{
    std::thread( server::start ).detach( );

    while ( true )
    {
        Sleep( 1000 );
        server::send( "module heartbeat", server::operations::PRINT );
    }

    return 0;
}