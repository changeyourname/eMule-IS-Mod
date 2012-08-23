//this file is part of eMule IS Mod
//Copyright (C)2009 IS Mod Team
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

#include "StdAfx.h"
#include <unordered_map>
#include <fstream>
#include <locale>
#include <codecvt>
#include "l10n.h"

using namespace std;

namespace l10n {

#define LANG_MAX_LINE_LENGTH 4000

std::tr1::unordered_map<std::wstring, std::wstring> g_lang;

static wchar_t * prepare( wchar_t * str )
{
    static wchar_t * trimlist = L" \t\r\n";

    // left trim
    while ( str[0] ) {
        if ( nullptr != wcschr(trimlist, str[0]) ) {
            str++; // move to the next character
        } else {
            break;
        }
    }

    // empty string
    if ( 0 == str[0] ) 
        return str;
    
    // right trim
    size_t i = wcslen(str)-1;
    while ( i >= 0 ) {
        if ( nullptr != wcschr(trimlist, str[i]) ) {
            str[i--] = 0; // cut tail character
        } else {
            break;
        }
        
    }

    // empty string
    if ( 0 == str[0] ) 
        return str;

    // replace all "\n" to real \r\n
    i = 0;
    while ( str[i] ) {
        if ( str[i] == L'\\' && str[i+1] == L'n' ) {
            str[i] = '\r';
            str[i+1] = '\n';
            i += 2;
        } else {
            i++;
        }
    }

    return str;
}

/**
 * @param file      path to file with l10n
 * @return          true on success
 */
bool loadLang( const wchar_t * file )
{
    wifstream stream;

    stream.open(file, ios_base::in );
    if ( stream.fail() )
        return false;

    // clear previously loaded language
    g_lang.clear();

    locale utf8_locale( locale(""), new codecvt_utf8<wchar_t, 0x10ffff, consume_header>);
    stream.imbue( utf8_locale );
    
    wchar_t line[LANG_MAX_LINE_LENGTH];
    bool wantKey = true;
    wstring key;

    while ( stream.good() ) {
        line[0] = L'0';
        stream.getline(line, LANG_MAX_LINE_LENGTH);
        wchar_t * cleanLine = prepare(line);
        size_t length = wcslen(cleanLine);

        if ( (length > 0) && (L';' != cleanLine[0]) ) {
            if ( wantKey && (L'[' == cleanLine[0]) && (L']' == cleanLine[length-1]) ) {
                wantKey = false;
                cleanLine[length-1] = 0; // trim right ]
                cleanLine++; // trim left
                cleanLine = prepare(cleanLine);
                key = cleanLine;
            } else if ( !wantKey ) {
                if ( key.size() ) {
                    g_lang[key] = cleanLine;
                    //clear key info
                    key.clear();
                    wantKey = true;
                }
            }
        }
    }

    stream.close();
    return true;
}

/**
 * @param text      text to translate
 * @return          translated text or 
 */

const wchar_t * translate( const wchar_t * text )
{
    auto it = g_lang.find(text);
    if ( it != g_lang.end() ) {
        return it->second.c_str();
    } else {
        return text;
    }
}

}