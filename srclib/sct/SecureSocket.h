//this file is part of eMule IS Mod
//Copyright (C)2009 eMule IS Mod
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#pragma once;

#ifndef __SECURE_SOCKET_H__
#define __SECURE_SOCKET_H__

#include <OpenSSl/ssl.h>

class SecureSocket
{
public:
    SecureSocket();
    ~SecureSocket();

    bool create( const char* bindHost = 0, uint16_t bindPort = 0 );
    bool connect( const char* host, uint16_t port );
    int read( void* buf, int length );
    int write( const void* data, int length );
    void close();
    static void loadSsl();
    static void unloadSsl();
    static in_addr resolve( const char* host );

private:

    static bool m_isInitialized; // SSL initialization 
    SOCKET m_sock;
    // ssl stuff
    SSL* m_ssl;
    SSL_CTX* m_ctx;
};

#endif // #ifndef __SECURE_SOCKET_H__