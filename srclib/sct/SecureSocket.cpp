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

#include <stdafx.h>
#include <OpenSSL/engine.h>
#include <OpenSSL/ssl.h>
#include <OpenSSL/err.h>
#include "SecureSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool SecureSocket::m_isInitialized = false;

SecureSocket::SecureSocket()
:   m_ssl(0),
    m_ctx(0),
    m_sock(0)
{
}

SecureSocket::~SecureSocket()
{
    close();
}

// init ssl
void SecureSocket::loadSsl()
{
    if ( m_isInitialized )
        return;
    SSL_load_error_strings();
    SSL_library_init();
    m_isInitialized = true;
}

// free ssl
void SecureSocket::unloadSsl()
{
    if ( !m_isInitialized )
        return;
    ENGINE_cleanup();
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    m_isInitialized = false;
}

bool SecureSocket::create( const char* bindHost /*= 0*/, uint16_t bindPort /*= 0*/ )
{
    m_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(m_sock == INVALID_SOCKET)
        return false;

    if ( bindHost )
    {
        struct sockaddr_in bind_addr;    

        memset(&bind_addr, 0, sizeof(bind_addr));
        bind_addr.sin_family = AF_INET;
        bind_addr.sin_port = htons(bindPort);
        bind_addr.sin_addr.s_addr = inet_addr(bindHost);

        if( !bind_addr.sin_addr.s_addr )
            return false;
        
        if ( bind(m_sock, (sockaddr*)&bind_addr, sizeof(bind_addr)) != 0 )
            return false;
    }

    return true;
}

bool SecureSocket::connect( const char* host, uint16_t port )
{
    ASSERT(host);
    
    const SSL_METHOD *method;

    if((method = SSLv23_client_method()) == NULL)
        return false;

    if((m_ctx = SSL_CTX_new(method)) == NULL)
        return false;

    struct sockaddr_in addr;    

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = resolve(host);

    if(addr.sin_addr.s_addr == NULL)
        return false;
    
    if(::connect(m_sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        return false;

    if((m_ssl = SSL_new(m_ctx)) == NULL)
        return false;

    if(SSL_set_fd(m_ssl, m_sock) == 0)
        return false;

    if(SSL_connect(m_ssl) != 1)
        return false;

    return true;
}

int SecureSocket::write( const void* data, int length )
{
    ASSERT(m_ssl);
    ASSERT(data);

    return SSL_write(m_ssl, data, length);
}

int SecureSocket::read( void* buf, int length )
{
    ASSERT(m_ssl);
    ASSERT(buf);

    int cur_len = SSL_read(m_ssl, buf, length);
    if(cur_len > 0)
    {
        return cur_len;
    }else if(SSL_get_error(m_ssl, cur_len) == SSL_ERROR_ZERO_RETURN)
        return 0;
    else
        return -1;
}

void SecureSocket::close()
{
    if ( m_ssl != NULL  ) { SSL_free(m_ssl);     m_ssl  = NULL; }
    if ( m_ctx != NULL  ) { SSL_CTX_free(m_ctx); m_ctx  = NULL; }
    if ( m_sock != NULL ) { closesocket(m_sock); m_sock = NULL; }
}

// резолвим в хостнейм или ИП в машинный формат
in_addr SecureSocket::resolve( const char* host )
{
    ASSERT(host);

    in_addr ret;
    ret.s_addr = NULL;
    
    if (host == NULL)
        return ret;

    ret.s_addr = inet_addr(host);

    if (ret.s_addr == INADDR_NONE)
    {
        LPHOSTENT lphost;
        lphost = gethostbyname(host);
        if (lphost != NULL)
            ret.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
        else
            ret.s_addr = NULL;
    }

    return ret;
}