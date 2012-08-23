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

#pragma once

#include "TreeOptionsCtrlEx.h"

class CSkinsListCtrl;

class CPPgIsMod : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgIsMod)

public:
	CPPgIsMod();

	void	Localize(void);

protected:
	CComboBox m_typesel;
	
	bool	m_bInitializedTreeOpts;
	CTreeOptionsCtrlEx	m_ctrlTreeOptions;

// options start

	// show popup delay
	HTREEITEM	m_htiPopupDelay;
	int			m_iPopupDelay;

	// friend slot for all friends
	HTREEITEM	m_htiFriendSlotsForAllFriends;
	BOOL		m_bFriendSlotsForAllFriends;

	// chat send method
	HTREEITEM	m_htiChatSendMethod;
	HTREEITEM	m_htiChat_SendOnSingleEnter;
	HTREEITEM	m_htiChat_SendOnDoubleEnter;
	int			m_iChatSendMethod;

#ifdef _DEBUG
	// don't use IS74 auth
	HTREEITEM	m_htiDontUseAuth;
	BOOL		m_bDontUseAuth;

	// server packet logger
	HTREEITEM	m_htiServerPacketLogger;
	BOOL		m_bServerPacketLogger;
#endif

	// delay before start emule (ms)
	HTREEITEM	m_htiStartDelay;
	int			m_iStartDelay;

	// upload slots count
	HTREEITEM	m_htiUploadSlots;
	int			m_iUploadSlots;

	// show extended info in title
	HTREEITEM	m_htiShowExtendedInfoInTitle;
	BOOL m_bShowExtendedInfoInTitle;

	// use client colors
	HTREEITEM	m_htiUseClientColors;
	BOOL m_bUseClientColors;

	// save search params
	HTREEITEM	m_htiSaveSearchParams;
	BOOL m_bSaveSearchParams;

	// random skins at startup
	HTREEITEM	m_htiUseRandomSkins;
	BOOL m_bUseRandomSkins;

	// always show tray icon
	HTREEITEM 	m_htiAlwaysShowTrayIcon;
	BOOL m_bAlwaysShowTrayIcon;

	CEdit m_editHost;
	CEdit m_editLogin;
	CEdit m_editPasswd;

// options end
	
	CTabCtrl m_tabPrefs;
	CString  m_sTBSkinsDir;
	CString	 m_sSkinsDir;
	CSkinsListCtrl* m_listTBSkins;
	CSkinsListCtrl* m_listSkins;
	CEdit m_editSkinsDir;
	// End MoNKi

	void ChangeTab(int nitem);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange() { SetModified(); }
	afx_msg void OnEnChangeEditHost() { SetModified(); }
	afx_msg void OnEnChangeEditLogin() { SetModified(); }
	afx_msg void OnEnChangeEditPasswd() { SetModified(); }
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnTcnSelchangeTabEmulespanaPrefs(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnSkinsDir();
	afx_msg void OnBnClickedBtnSkinsReload();
	afx_msg void OnEnChangeEditSkinsDir();
	afx_msg void OnSysColorChange();
	afx_msg void OnLvnItemchangedListSkins(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedListTbSkins(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnSelchangedExtOpts(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedModWordfilter();

private:
    HICON m_icoBrowse;
};