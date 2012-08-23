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

#define SCT_LISTEN_PORT             43434 // IANA info: # 43192-43439 Unassigned
#define SCT_IS74_ADDR               _T(SCT_IS74_ADDR_A)
#define SCT_IS74_ADDR_A             "is74.ru"
#define SCT_SERVER_ADDR             L"tcs.is74.ru"
#define SCT_WEB_PATH                L"/"
#define SCT_AUTH_SRV                L"el.is74.ru"
#define SCT_SERVER_PORT             443
#define SCT_FIRST_DELAY             10*60*1000  // 10min
#define SCT_VERSION                 L"0.26"
#define SCT_ICMP_TIMEOUT            1000 // 1 sec
#define SCT_TEST_DATA               "SCT_IS74_TEST"

#define SCT_VIDEOSERVER_HOST		L"news.is74.ru"
#define SCT_VIDEOSERVER_PORT		80

#define SCT_IPF_URL                 "http://tcs.is74.ru/ipfilter.dat"

#define SCT_PIS_URL                 "http://pis.is74.ru/message_emule.php?source=emule"
#define SCT_PIS_URL_ACCEPT          "http://pis.is74.ru/message_emule.php?source=emule&submit=1"
#define SCT_PIS_INTERVAL			1*60*60*1000 // 1 час

// options
#define SCT_OPTIONS_REG_LOCATION   _T("Software\\eMule IS Mod")
#define SCT_OPTIONS_SCT_LAST_UPDATE_CHECK_TIME _T("SctLastUpdateCheckTime")