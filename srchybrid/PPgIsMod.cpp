//this file is part of eMule IS Mod
//Copyright (C) 2011 IS Mod Team
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

#include "stdafx.h"
#include <CxImage\ximage.h>
#include "TempIconLoader.h"

#include "emule.h"
#include "PPgIsMod.h"
#include "SkinsListCtrl.h"
#include "OtherFunctions.h"
#include "UserMsgs.h"
#include "eMuleDlg.h"
#include "ini2.h"
#include "WordFilterDlg.h"
#include "opcodes.h"

IMPLEMENT_DYNAMIC(CPPgIsMod, CPropertyPage)

CPPgIsMod::CPPgIsMod()
	: CPropertyPage(IDD_MOD_PPG_ISMOD)
	, m_ctrlTreeOptions(ILC_COLOR32)	
{
    m_icoBrowse = NULL;

	m_bInitializedTreeOpts = false;

	m_htiStartDelay = NULL;
	m_iStartDelay = 0;

	m_htiUploadSlots = NULL;
	m_iUploadSlots = 0;

	m_htiPopupDelay = NULL;
	m_iPopupDelay = 0;

	m_htiFriendSlotsForAllFriends = NULL;
	m_bFriendSlotsForAllFriends = false;

	m_htiChat_SendOnSingleEnter = NULL;
	m_htiChat_SendOnDoubleEnter = NULL;
	m_iChatSendMethod = SEND_ON_DOUBLE_ENTER;

#ifdef _DEBUG
	m_htiDontUseAuth = NULL;
	m_bDontUseAuth = false;

	m_htiServerPacketLogger = NULL;
	m_bServerPacketLogger = false;
#endif

	m_htiShowExtendedInfoInTitle = NULL;
	m_bShowExtendedInfoInTitle = false;

	m_htiUseClientColors = NULL;
	m_bUseClientColors = false;

	m_htiSaveSearchParams = NULL;
	m_bSaveSearchParams = false;

	m_htiAlwaysShowTrayIcon = NULL;
	m_bAlwaysShowTrayIcon = false;

    m_htiUseRandomSkins = NULL;
	m_bUseRandomSkins = false;

	m_listTBSkins = NULL;
	m_listSkins = NULL;
}

void CPPgIsMod::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXT_OPTS, m_ctrlTreeOptions);
	if ( !m_bInitializedTreeOpts )
	{
		int iImgChat = 8;
		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgChat =piml->Add(CTempIconLoader(_T("Chat")));
		}


		m_htiPopupDelay = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MOD_POPUP_DELAY), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiPopupDelay, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiFriendSlotsForAllFriends = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MOD_FRSLOT4ALLFR), TVI_ROOT, m_bFriendSlotsForAllFriends);
		m_htiChatSendMethod = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MOD_CHAT_SEND_METHOD), iImgChat, TVI_ROOT);
		m_htiChat_SendOnDoubleEnter = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_MOD_SND_MSG_ON_DBL), m_htiChatSendMethod, m_iChatSendMethod==SEND_ON_DOUBLE_ENTER);
		m_htiChat_SendOnSingleEnter = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_MOD_SND_MSG_ON_SNGL), m_htiChatSendMethod, m_iChatSendMethod==SEND_ON_SINGLE_ENTER);
#ifdef _DEBUG
		m_htiDontUseAuth = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MOD_DISABLEAUTH), TVI_ROOT, m_bDontUseAuth);
		m_htiServerPacketLogger = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MOD_SERVER_PACKET_LOGGER), TVI_ROOT, m_bServerPacketLogger);
#endif
		m_htiUseClientColors = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MOD_CLIENTCOLORS), TVI_ROOT, m_bUseClientColors);
		m_htiShowExtendedInfoInTitle  = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MOD_EXTENDTITLE), TVI_ROOT, m_bShowExtendedInfoInTitle);
		m_htiSaveSearchParams  = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MOD_SAVESEARCHPARAMS), TVI_ROOT, m_bSaveSearchParams);
		m_htiAlwaysShowTrayIcon = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MOD_TRAYICON), TVI_ROOT, m_bAlwaysShowTrayIcon);
		m_htiUseRandomSkins  = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MOD_RANDOMSKINS), TVI_ROOT, m_bUseRandomSkins);

		m_htiStartDelay = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MOD_STARTDELAY), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiStartDelay, RUNTIME_CLASS(CNumTreeOptionsEdit));

		CString uploadSlotsString;
		uploadSlotsString.Format(GetResString(IDS_MOD_UPLOADSLOTS), MAX_UP_CLIENTS_ALLOWED, MIN_UP_CLIENTS_ALLOWED + 1);
		m_htiUploadSlots = m_ctrlTreeOptions.InsertItem(uploadSlotsString, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiUploadSlots, RUNTIME_CLASS(CNumTreeOptionsEdit));


		m_ctrlTreeOptions.SelectItem(m_ctrlTreeOptions.GetRootItem());
		m_bInitializedTreeOpts = true;
	}

	DDX_Control(pDX, IDC_MOD_TAB_EMULESPANA_PREFS, m_tabPrefs);
	DDX_Control(pDX, IDC_MOD_LIST_SKINS, *m_listSkins);
	DDX_Control(pDX, IDC_MOD_LIST_TB_SKINS, *m_listTBSkins);
	DDX_Control(pDX, IDC_MOD_EDIT_SKINS_DIR, m_editSkinsDir);

	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiStartDelay, m_iStartDelay);
	DDV_MinMaxInt(pDX, m_iStartDelay, 0, 60);

	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiUploadSlots, m_iUploadSlots);
	DDV_MinMaxInt(pDX, m_iUploadSlots, MIN_UP_CLIENTS_ALLOWED + 1 , MAX_UP_CLIENTS_ALLOWED);

	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiPopupDelay, m_iPopupDelay);
	DDV_MinMaxInt(pDX, m_iPopupDelay, 0, 60000);

	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiFriendSlotsForAllFriends, m_bFriendSlotsForAllFriends);

	m_ctrlTreeOptions.Expand(m_htiChatSendMethod, TVE_EXPAND);
	DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiChatSendMethod, m_iChatSendMethod);


#ifdef _DEBUG
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiDontUseAuth, m_bDontUseAuth);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiServerPacketLogger, m_bServerPacketLogger);
#endif
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiShowExtendedInfoInTitle, m_bShowExtendedInfoInTitle);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUseClientColors, m_bUseClientColors);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiSaveSearchParams, m_bSaveSearchParams);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiAlwaysShowTrayIcon, m_bAlwaysShowTrayIcon);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUseRandomSkins, m_bUseRandomSkins);

}


BEGIN_MESSAGE_MAP(CPPgIsMod, CPropertyPage)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_WM_MEASUREITEM()
	ON_WM_SYSCOLORCHANGE()
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	//ON_EN_CHANGE(IDC_MOD_EDIT_HOST, OnEnChangeEditHost)
	ON_EN_CHANGE(IDC_MOD_EDIT_LOGIN, OnEnChangeEditLogin)
	ON_EN_CHANGE(IDC_MOD_EDIT_PASSWD, OnEnChangeEditPasswd)
	ON_EN_CHANGE(IDC_MOD_EDIT_SKINS_DIR, OnEnChangeEditSkinsDir)
	ON_BN_CLICKED(IDC_MOD_WORDFILTER, OnBnClickedModWordfilter)
	ON_BN_CLICKED(IDC_MOD_BTN_SKINS_DIR, OnBnClickedBtnSkinsDir)
	ON_BN_CLICKED(IDC_MOD_BTN_SKINS_RELOAD, OnBnClickedBtnSkinsReload)
	ON_NOTIFY(TVN_SELCHANGED, IDC_EXT_OPTS, OnTvnSelchangedExtOpts)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_MOD_LIST_SKINS, OnLvnItemchangedListSkins)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_MOD_LIST_TB_SKINS, OnLvnItemchangedListTbSkins)
	ON_NOTIFY(TCN_SELCHANGE, IDC_MOD_TAB_EMULESPANA_PREFS, OnTcnSelchangeTabEmulespanaPrefs)
END_MESSAGE_MAP()

void CPPgIsMod::Localize()
{
	if ( m_hWnd == NULL )
		return;

	SetWindowText(GetResString(IDS_MOD_ISMODTWEAKS));

	TCITEM tabItem;
	CString tmpString;
	tmpString = GetResString(IDS_EM_PREFS); 
	tmpString.Remove('&'); 
	tabItem.mask = TCIF_TEXT;
	tabItem.pszText = tmpString.GetBuffer();
	m_tabPrefs.SetItem(1, &tabItem);
	tmpString.ReleaseBuffer();
	tmpString = GetResString(IDS_TOOLBARSKINS);
	tabItem.pszText = tmpString.GetBuffer();
	m_tabPrefs.SetItem(2, &tabItem);
	tmpString.ReleaseBuffer();
	tmpString = GetResString(IDS_SKIN_PROF);
	tabItem.pszText = tmpString.GetBuffer();
	m_tabPrefs.SetItem(3, &tabItem);
	tmpString.ReleaseBuffer();

	tmpString = GetResString(IDS_MOD_AUTHORIZATION);
	tabItem.pszText = tmpString.GetBuffer();
	m_tabPrefs.SetItem(0, &tabItem);
	tmpString.ReleaseBuffer();

	GetDlgItem(IDC_MOD_BTN_SKINS_RELOAD)->SetWindowText(GetResString(IDS_SF_RELOAD));
	if(m_tabPrefs.GetCurSel() == 1)
		SetDlgItemText(IDC_MOD_SKINS_LBL, GetResString(IDS_SELECTTOOLBARBITMAPDIR));
	else if (m_tabPrefs.GetCurSel() == 2)
		SetDlgItemText(IDC_MOD_SKINS_LBL, GetResString(IDS_SEL_SKINDIR));
		
	m_listTBSkins->Localize();
	m_listSkins->Localize();

	if(m_htiStartDelay)
		m_ctrlTreeOptions.SetEditLabel(m_htiStartDelay, GetResString(IDS_MOD_STARTDELAY));

	m_ctrlTreeOptions.SetItemText(m_htiShowExtendedInfoInTitle, GetResString(IDS_MOD_ADDITINFO));
	m_ctrlTreeOptions.SetItemText(m_htiUseClientColors, GetResString(IDS_MOD_CLIENTCOLORS));
	m_ctrlTreeOptions.SetItemText(m_htiSaveSearchParams, GetResString(IDS_MOD_SAVESEARCHPARAMS));
	m_ctrlTreeOptions.SetItemText(m_htiAlwaysShowTrayIcon, GetResString(IDS_MOD_TRAYICON));
	m_ctrlTreeOptions.SetItemText(m_htiUseRandomSkins, GetResString(IDS_MOD_RANDOMSKINS));

	//SetDlgItemText(IDC_MOD_STATIC_HOST, GetResString(IDS_MOD_AUTHHOST));
	SetDlgItemText(IDC_MOD_STATIC_LOGIN, GetResString(IDS_MOD_LOGIN));
	SetDlgItemText(IDC_MOD_STATIC_PASSWD, GetResString(IDS_MOD_PASSWORD));
	SetDlgItemText(IDC_MOD_WORDFILTER, GetResString(IDS_MOD_WORDFILTER));
}

LRESULT CPPgIsMod::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam == IDC_EXT_OPTS)
		SetModified();

	return 0;
}

void CPPgIsMod::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;

	m_htiPopupDelay = NULL;
	m_htiFriendSlotsForAllFriends = NULL;
	m_htiChatSendMethod = NULL;
	m_htiChat_SendOnDoubleEnter = NULL;
	m_htiChat_SendOnSingleEnter = NULL;
#ifdef _DEBUG
	m_htiDontUseAuth = NULL;
	m_htiServerPacketLogger = NULL;
#endif
	m_htiStartDelay = NULL;
	m_htiUploadSlots = NULL;
	m_htiShowExtendedInfoInTitle = NULL;
	m_htiUseClientColors = NULL;
	m_htiUseRandomSkins = NULL;
	m_htiSaveSearchParams = NULL;
	m_htiAlwaysShowTrayIcon = NULL;	

    delete m_listSkins; m_listSkins = NULL;
    delete m_listTBSkins; m_listTBSkins = NULL;
    
    CPropertyPage::OnDestroy();

    if (m_icoBrowse)
    {
        VERIFY( DestroyIcon(m_icoBrowse) );
        m_icoBrowse = NULL;
    }
}

BOOL CPPgIsMod::OnApply()
{
	//GetDlgItem(IDC_MOD_EDIT_HOST)->GetWindowText(thePrefs.m_strAuthServ);
	GetDlgItem(IDC_MOD_EDIT_LOGIN)->GetWindowText(thePrefs.m_strAuthLogin);
	GetDlgItem(IDC_MOD_EDIT_PASSWD)->GetWindowText(thePrefs.m_strAuthPasswd);
	
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	bool bRestartApp = false;

	CString sToolbarSkin, sIniSkin;

	thePrefs.SetToolbarBitmapFolderSettings(m_sTBSkinsDir);
	thePrefs.SetSkinProfileDir(m_sSkinsDir);

	sToolbarSkin = m_listTBSkins->GetSelectedSkin();
	sIniSkin = m_listSkins->GetSelectedSkin();

	if(sToolbarSkin.MakeLower() != CString(thePrefs.GetToolbarBitmapSettings()).MakeLower()){
		m_listTBSkins->SelectCurrentSkin();
	}
	
	if(sIniSkin.MakeLower() != CString(thePrefs.GetSkinProfile()).MakeLower()){
		m_listSkins->SelectCurrentSkin();
	}

	if(m_iStartDelay < 0)
		m_iStartDelay = 0;
	if(m_iStartDelay > 60)
		m_iStartDelay = 255;

	if(m_iPopupDelay < 0)
		m_iPopupDelay = 0;
	if(m_iPopupDelay > 60000)
		m_iPopupDelay = 4000;
	CIni ini(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("Notifier.ini"), _T("Config"));
	ini.WriteInt(_T("TimeToStay"), m_iPopupDelay);

	thePrefs.m_bFriendSlotForAllFriends = m_bFriendSlotsForAllFriends!=0;
	thePrefs.m_iChatSendMethod  = (EChatSendMethod)m_iChatSendMethod;
#ifdef DEBUG
	thePrefs.m_bDontUseAuth             = m_bDontUseAuth!=0;
	thePrefs.m_bServerPacketLogger      = m_bServerPacketLogger!=0;
#endif
	thePrefs.m_iMaxUploadSlots          = m_iUploadSlots;
	thePrefs.m_bShowExtendedInfoInTitle = m_bShowExtendedInfoInTitle!=0;
	thePrefs.m_bUseClientColors         = m_bUseClientColors!=0;
	thePrefs.m_bSaveSearchParams        = m_bSaveSearchParams!=0;
	thePrefs.m_bAlwaysShowTrayIcon      = m_bAlwaysShowTrayIcon!=0;
	thePrefs.m_bUseRandomSkins          = m_bUseRandomSkins!=0;
	thePrefs.m_iStartDelay              = m_iStartDelay;

	CSliderCtrl *sliderUpdate = (CSliderCtrl*)GetDlgItem(IDC_MOD_HASHSPEED);
	thePrefs.m_iHashingSpeed = - 3 * (sliderUpdate->GetPos() - 10);

	if (m_bAlwaysShowTrayIcon == TRUE)
		theApp.emuledlg->TrayShow();

	if (bRestartApp)
		AfxMessageBox(GetResString(IDS_SETTINGCHANGED_RESTART));

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

BOOL CPPgIsMod::OnInitDialog()
{
	m_iStartDelay = thePrefs.m_iStartDelay;
	m_iUploadSlots = thePrefs.m_iMaxUploadSlots;
	CIni ini(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("Notifier.ini"), _T("Config"));
	m_iPopupDelay = ini.GetInt(_T("TimeToStay"), 4000);	
	m_bFriendSlotsForAllFriends = thePrefs.m_bFriendSlotForAllFriends;
	m_iChatSendMethod = thePrefs.m_iChatSendMethod;
#ifdef DEBUG
	m_bDontUseAuth = thePrefs.m_bDontUseAuth;
	m_bServerPacketLogger = thePrefs.m_bServerPacketLogger;
#endif
	m_bShowExtendedInfoInTitle = thePrefs.m_bShowExtendedInfoInTitle;
	m_bUseClientColors = thePrefs.m_bUseClientColors;
	m_bSaveSearchParams = thePrefs.m_bSaveSearchParams;
	m_bAlwaysShowTrayIcon = thePrefs.m_bAlwaysShowTrayIcon;
	m_bUseRandomSkins = thePrefs.m_bUseRandomSkins;
	

    m_listTBSkins = new CSkinsListCtrl;
    m_listTBSkins->SetPrefsKey(_T("PpgISModToolbarSkins"));

    m_listSkins = new CSkinsListCtrl;
    m_listSkins->SetPrefsKey(_T("PpgISModSkinsProfiles"));
    
    CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	m_tabPrefs.InsertItem(0, GetResString(IDS_MOD_AUTHORIZATION));
	CString tmpString = GetResString(IDS_EM_PREFS); 
	tmpString.Remove(_T('&')); 
	m_tabPrefs.InsertItem(1,tmpString);
	m_tabPrefs.InsertItem(2,GetResString(IDS_TOOLBARSKINS));
	m_tabPrefs.InsertItem(3,GetResString(IDS_SKIN_PROF));

    m_sTBSkinsDir = thePrefs.GetToolbarBitmapFolderSettings();
    m_listTBSkins->Init();
    m_listTBSkins->LoadToolBars(m_sTBSkinsDir);

    m_sSkinsDir = thePrefs.GetSkinProfileDir();
    m_listSkins->Init();
    m_listSkins->LoadSkins(m_sSkinsDir);

    AddBuddyButton(GetDlgItem(IDC_MOD_EDIT_SKINS_DIR)->m_hWnd, ::GetDlgItem(m_hWnd, IDC_MOD_BTN_SKINS_DIR));
    SetButtonIcon(::GetDlgItem(m_hWnd, IDC_MOD_BTN_SKINS_DIR), m_icoBrowse);

    ChangeTab(0);

	CSliderCtrl *sliderUpdate = (CSliderCtrl*)GetDlgItem(IDC_MOD_HASHSPEED);
	sliderUpdate->SetRange(0, 10, true);
	int nSleepTime = thePrefs.m_iHashingSpeed;
	if (nSleepTime < 0) nSleepTime = 0;
	else if (nSleepTime > 30) nSleepTime = 30;
	sliderUpdate->SetPos(-nSleepTime/3 + 10);
	CString text;
	if (nSleepTime == 0) text = _T("Без ограничений");
	else
	if (nSleepTime > 0 && nSleepTime < 10)
		text = _T("Высокая скорость хеширования файлов");
	else if (nSleepTime >= 10 && nSleepTime < 20) 
		text = _T("Средняя скорость хеширования файлов");
	else text = _T("Низкая скорость хеширования файлов");
	GetDlgItem(IDC_MOD_HASHSPEED_TXT)->SetWindowText(text);

	//GetDlgItem(IDC_MOD_EDIT_HOST)->SetWindowText(thePrefs.m_strAuthServ);
    GetDlgItem(IDC_MOD_EDIT_LOGIN)->SetWindowText(thePrefs.m_strAuthLogin);
    GetDlgItem(IDC_MOD_EDIT_PASSWD)->SetWindowText(thePrefs.m_strAuthPasswd);

	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgIsMod::OnCommand(WPARAM wParam,LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}

	return CPropertyPage::OnCommand(wParam, lParam);
}

BOOL CPPgIsMod::OnKillActive()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

void CPPgIsMod::ChangeTab(int nitem)
{
	switch(nitem){
		case 1:
			//GetDlgItem(IDC_MOD_EDIT_HOST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_EDIT_LOGIN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_EDIT_PASSWD)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_MOD_STATIC_HOST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_STATIC_LOGIN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_STATIC_PASSWD)->ShowWindow(SW_HIDE);

			m_ctrlTreeOptions.ShowWindow(SW_SHOW);
			m_listTBSkins->ShowWindow(SW_HIDE);
			m_listSkins->ShowWindow(SW_HIDE);
			m_editSkinsDir.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_SKINS_LBL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_BTN_SKINS_DIR)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_BTN_SKINS_RELOAD)->ShowWindow(SW_HIDE);
			break;
		case 2:
			//GetDlgItem(IDC_MOD_EDIT_HOST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_EDIT_LOGIN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_EDIT_PASSWD)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_MOD_STATIC_HOST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_STATIC_LOGIN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_STATIC_PASSWD)->ShowWindow(SW_HIDE);

			m_ctrlTreeOptions.ShowWindow(SW_HIDE);
			m_listTBSkins->ShowWindow(SW_SHOW);
			m_listSkins->ShowWindow(SW_HIDE);
			m_editSkinsDir.SetWindowText(m_sTBSkinsDir);
			m_editSkinsDir.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MOD_SKINS_LBL)->SetWindowText(GetResString(IDS_SELECTTOOLBARBITMAPDIR));
			GetDlgItem(IDC_MOD_SKINS_LBL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MOD_BTN_SKINS_DIR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MOD_BTN_SKINS_RELOAD)->ShowWindow(SW_SHOW);
			break;
		case 3:
			//GetDlgItem(IDC_MOD_EDIT_HOST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_EDIT_LOGIN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_EDIT_PASSWD)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_MOD_STATIC_HOST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_STATIC_LOGIN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_STATIC_PASSWD)->ShowWindow(SW_HIDE);

			m_ctrlTreeOptions.ShowWindow(SW_HIDE);
			m_listTBSkins->ShowWindow(SW_HIDE);
			m_listSkins->ShowWindow(SW_SHOW);
			m_editSkinsDir.SetWindowText(m_sSkinsDir);
			m_editSkinsDir.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MOD_SKINS_LBL)->SetWindowText(GetResString(IDS_SEL_SKINDIR));
			GetDlgItem(IDC_MOD_SKINS_LBL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MOD_BTN_SKINS_DIR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MOD_BTN_SKINS_RELOAD)->ShowWindow(SW_SHOW);
			break;
		case 4:
			//GetDlgItem(IDC_MOD_EDIT_HOST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_EDIT_LOGIN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_EDIT_PASSWD)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_MOD_STATIC_HOST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_STATIC_LOGIN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_STATIC_PASSWD)->ShowWindow(SW_HIDE);

			m_ctrlTreeOptions.ShowWindow(SW_HIDE);
			m_listTBSkins->ShowWindow(SW_HIDE);
			m_listSkins->ShowWindow(SW_HIDE);
			m_editSkinsDir.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_SKINS_LBL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_BTN_SKINS_DIR)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_BTN_SKINS_RELOAD)->ShowWindow(SW_HIDE);
			break;

		case 0:
			//GetDlgItem(IDC_MOD_EDIT_HOST)->ShowWindow(SW_SHOW);
			//GetDlgItem(IDC_MOD_EDIT_HOST)->SetWindowText(thePrefs.m_strAuthServ);
			GetDlgItem(IDC_MOD_EDIT_LOGIN)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MOD_EDIT_LOGIN)->SetWindowText(thePrefs.m_strAuthLogin);
			GetDlgItem(IDC_MOD_EDIT_PASSWD)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MOD_EDIT_PASSWD)->SetWindowText(thePrefs.m_strAuthPasswd);
			//GetDlgItem(IDC_MOD_STATIC_HOST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MOD_STATIC_LOGIN)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MOD_STATIC_PASSWD)->ShowWindow(SW_SHOW);

			m_ctrlTreeOptions.ShowWindow(SW_HIDE);
			m_listTBSkins->ShowWindow(SW_HIDE);
			m_listSkins->ShowWindow(SW_HIDE);
			m_editSkinsDir.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_SKINS_LBL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_BTN_SKINS_DIR)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOD_BTN_SKINS_RELOAD)->ShowWindow(SW_HIDE);
			break;
		//miramir Stop
	}
}

void CPPgIsMod::OnTcnSelchangeTabEmulespanaPrefs(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	ChangeTab(m_tabPrefs.GetCurSel());

	*pResult = 0;
}

void CPPgIsMod::OnBnClickedBtnSkinsDir()
{
	TCHAR buffer[MAX_PATH] = {0};
	CString title;

	_tcsncpy(buffer, m_sTBSkinsDir.GetBuffer(), _countof(buffer));
	m_sTBSkinsDir.ReleaseBuffer();
	
	GetDlgItem(IDC_MOD_SKINS_LBL)->GetWindowText(title);
	if(SelectDir(GetSafeHwnd(),buffer,title.GetBuffer())){
		m_editSkinsDir.SetWindowText(buffer);
	}
}

void CPPgIsMod::OnBnClickedBtnSkinsReload()
{
	switch(m_tabPrefs.GetCurSel()){
		case 1:
			m_listTBSkins->LoadToolBars(m_sTBSkinsDir);
			break;
		case 2:
			m_listSkins->LoadSkins(m_sSkinsDir);
			break;
	}
}

void CPPgIsMod::OnEnChangeEditSkinsDir()
{
	switch(m_tabPrefs.GetCurSel()){
		case 1:
			m_editSkinsDir.GetWindowText(m_sTBSkinsDir);
			break;
		case 2:
			m_editSkinsDir.GetWindowText(m_sSkinsDir);
			break;
	}
	SetModified();
}

void CPPgIsMod::OnSysColorChange()
{
	CPropertyPage::OnSysColorChange();

	m_listTBSkins->SendMessage(WM_SYSCOLORCHANGE);
	m_listSkins->SendMessage(WM_SYSCOLORCHANGE);
}

void CPPgIsMod::OnLvnItemchangedListSkins(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	/*
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*/

	SetModified();

	*pResult = 0;
}

void CPPgIsMod::OnLvnItemchangedListTbSkins(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	/*
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*/

	SetModified();

	*pResult = 0;
}

void CPPgIsMod::OnTvnSelchangedExtOpts(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	/*
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*/

	SetModified();

	*pResult = 0;
}

void CPPgIsMod::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);

	if (pScrollBar==GetDlgItem(IDC_MOD_HASHSPEED)) {
		CSliderCtrl* slider =(CSliderCtrl*)pScrollBar;
		CString text;
		int temp = - 3 * (slider->GetPos() - 10);
		if (temp == 0) text = _T("Без ограничений");
		else
		if (temp > 0 && temp < 10)
			text = _T("Высокая скорость хеширования файлов");
		else if (temp >= 10 && temp < 20) 
			text = _T("Средняя скорость хеширования файлов");
		else text = _T("Низкая скорость хеширования файлов");
			GetDlgItem(IDC_MOD_HASHSPEED_TXT)->SetWindowText(text);
	}
	UpdateData(false); 
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgIsMod::OnBnClickedModWordfilter()
{
	CWordFilterDlg wordsDlg;
	wordsDlg.DoModal();
}