//this file is part of eMule IS Mod
//Copyright (C)2011 eMule IS Mod Team
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <assert.h>
#include <process.h>

#include <curl/curl.h>
#include "emhelper.h"

using namespace std;

namespace emCurl {

/**
 * @param code         cURL error code
 * @return             error text representation
 */
const char EMHELPER_API * formatError( int code )
{
	return curl_easy_strerror( static_cast<CURLcode>(code) );
}

struct AsyncRequestInfo {
    HWND hwnd;
    UINT msg;
    string url;
    string postData;
    ostream * outStream;
};

static size_t streamWriteCallback( void *ptr, size_t size, size_t nmemb, void *data )
{
    size_t realsize = size * nmemb;
    ostream & stream = *(ostream*)data;

    stream.write((const char *)ptr, realsize);

    return realsize;
}
    
/**
 * @param url          requested URL
 * @param postData     post data or null
 * @param outStream    output stream
 * @return             CURLCode
 */
int EMHELPER_API httpRequest( const string & url, const string & postData , ostream & outStream )
{
	CURL *curl_handle;
	CURLcode ret;

    outStream.flush();
    outStream.seekp(0, ostream::beg);

    if ( !outStream.good() )
        return CURLE_WRITE_ERROR; 
    
    curl_handle = curl_easy_init();

	if ( NULL == curl_handle )
		return CURLE_FAILED_INIT;

	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	if ( !postData.empty() ) {
		curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postData.c_str());
	}
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, streamWriteCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&outStream);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "eMule IS Mod");
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);

	ret = curl_easy_perform(curl_handle);

    outStream.flush();

	if ( CURLE_OK != ret ) {
		curl_easy_cleanup(curl_handle);
		return ret;
	}

	long ret_code = 0;
	ret = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &ret_code);
	if ( CURLE_OK != ret || 200 != ret_code ) {
		curl_easy_cleanup(curl_handle);
		return CURLE_HTTP_RETURNED_ERROR;
	}

	curl_easy_cleanup(curl_handle);

	return CURLE_OK;
}

/**
 * async httpRequest thread procedure
 */
static unsigned __stdcall httpRequestAsyncThread( void *data )
{
    assert( nullptr != data );
    AsyncRequestInfo *ari = static_cast<AsyncRequestInfo*>(data);
    
    ostream * outStream = ari->outStream;
    int ret = httpRequest( ari->url, ari->postData, *outStream);
    PostMessage(ari->hwnd, ari->msg, (WPARAM)ret, (LPARAM)outStream);
    delete ari;
    
    return 0;
}

/**
 * Creates thread for executing http request synchronously in it
 * Posts message @msg to window @hwnd ( WPARAM - curl error code, LPARAM - output stream )
 * @param url          requested URL
 * @param postData     post data or null
 * @param outStream    output stream
 * @return             true on success
 */
bool EMHELPER_API httpRequestAsync( const std::string & url, const std::string & postData, ostream * outStream, HWND hwnd, UINT msg )
{
    assert( nullptr != outStream );

    if ( !outStream->good() )
        return false;

    AsyncRequestInfo *ari = new AsyncRequestInfo;

    ari->hwnd = hwnd;
    ari->msg = msg;
    ari->url = url;
    ari->postData = postData;
    ari->outStream = outStream; 

    unsigned int threadId;
    uintptr_t handle = _beginthreadex(NULL, 0, &httpRequestAsyncThread, ari, CREATE_SUSPENDED, &threadId);

    if ( (0 == handle) || (uintptr_t(-1) == handle) ) {
        delete ari;
        return false;
    }

    ResumeThread(reinterpret_cast<HANDLE>(handle));

    return true;
}

} // namespace Curl