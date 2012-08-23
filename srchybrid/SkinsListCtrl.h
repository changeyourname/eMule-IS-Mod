// emulEspaña: Added by MoNKi [MoNKi: -Skin Selector-]

#pragma once

#include "CXImage\ximage.h"
#include "MuleListCtrl.h"

// CSkinsListCtrl

class CSkinsListCtrl : public CMuleListCtrl
{
	DECLARE_DYNAMIC(CSkinsListCtrl)

public:
	CSkinsListCtrl();
	virtual ~CSkinsListCtrl();

	void	Init();
	void	LoadToolBars(CString path = _T(""));
	void	LoadSkins(CString path = _T(""));
	void	Localize();
	CString GetSelectedSkin();
	void	SelectCurrentSkin();
	bool	AddFile(CString file);
	void	SetSortAscending(bool asc)		{ m_bOrderAscending = asc;  }
	bool	GetSortAscending()				{ return m_bOrderAscending; }

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	DECLARE_MESSAGE_MAP()
private:
	typedef struct{
		bool	bToolBar;
		CString name;
		CString toolbarFile;
		CString	iniFile;
		CxImage	*image;
	} SKIN_ITEM;

	typedef enum{
		MODE_NONE,
		MODE_TOOLBARS,
		MODE_SKINS
	} SKIN_MODE;

	CxImage*	GetToolBarPreview(CString toolbarFile, int width = -1);
	CxImage*	GetIniPreview(CString toolbarFile, int width = -1);
	CxImage*	GetSkinPreview(SKIN_ITEM &item, int width = -1);
	void		RemoveItems();

	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg	void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	enum{
		MP_APPLY_CURRENT_SKIN = 10000,
		MP_SELECT_SKIN_FILE,
		MP_GET_MORE_SKINS
	};

	CTypedPtrList<CPtrList, SKIN_ITEM*> m_skinsList;
	
	int m_iCurrent;
	bool m_bOrderAscending;
	SKIN_MODE m_iMode;
};


