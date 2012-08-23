// emulEspaña: Added by MoNKi [MoNKi: -Skin Selector-]

#include "stdafx.h"
#include "TempIconLoader.h"
#include "emule.h"
#include "emuledlg.h"
#include "SkinsListCtrl.h"
#include "memdc.h"
#include "OtherFunctions.h"
#include "MuleToolbarCtrl.h"
#include "SkinsListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	EMULTB_BASEEXT		_T("eMuleToolbar.kad02")
#define	EMULSKIN_BASEEXT	_T("eMuleSkin")

static const LPCTSTR _apszTBFiles[] = 
{
	_T("*.") EMULTB_BASEEXT _T(".bmp"),
	_T("*.") EMULTB_BASEEXT _T(".gif"),
	_T("*.") EMULTB_BASEEXT _T(".png")
};

static const LPCTSTR _apszSkinFiles[] = 
{
	_T("*.") EMULSKIN_BASEEXT _T(".ini"),
};

// CSkinsListCtrl

IMPLEMENT_DYNAMIC(CSkinsListCtrl, CListCtrl)
CSkinsListCtrl::CSkinsListCtrl()
{
	m_iCurrent = -1;
	m_bOrderAscending = true;
	m_iMode = MODE_NONE;
}

CSkinsListCtrl::~CSkinsListCtrl()
{
    RemoveItems();
}

void CSkinsListCtrl::Init()
{
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries

	//Gets the height
	TEXTMETRIC txtMetric;
	GetDC()->GetOutputTextMetrics(&txtMetric);

	ilDummyImageList.Create(1, txtMetric.tmHeight + 47,ILC_COLOR32|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);

	CRect rc;
	GetWindowRect(&rc);
	InsertColumn(0,_T("Skins"),LVCFMT_LEFT, rc.Width() - GetSystemMetrics(SM_CXVSCROLL) - (GetSystemMetrics(SM_CYBORDER)*2) - 2 ,0);

	SetSortArrow(0, m_bOrderAscending);
	SortItems(SortProc, 1 + (m_bOrderAscending ? 0:20));
}

BEGIN_MESSAGE_MAP(CSkinsListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CSkinsListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if( !theApp.IsRunning() )
		return;
	if( !lpDrawItemStruct->itemData){
		return;
	}
	
	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);

	//gets the item image and state info
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = lpDrawItemStruct->itemID;
	lvi.iSubItem = 0;
	lvi.stateMask = LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED | LVIS_GLOW; 
	GetItem(&lvi);

	BOOL bCtrlFocused = ((GetFocus() == this ) || (GetStyle() & LVS_SHOWSELALWAYS));
	BOOL bGlowing = ( lvi.state & LVIS_GLOW );
	if( odc && (lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED )){
		if(bCtrlFocused){
			odc->SetBkColor(m_crHighlight);
		}
		else if(bGlowing){
			odc->SetBkColor(m_crGlow);
		}
		else{
			odc->SetBkColor(m_crNoHighlight);
		}
	}
	else{
		if(bGlowing){
			odc->SetBkColor(m_crGlow);
		}
		else{
			odc->SetBkColor(m_crWindow);
		}
	}

	SKIN_ITEM* skinItem = (SKIN_ITEM*)lpDrawItemStruct->itemData;
	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	RECT cur_rec = lpDrawItemStruct->rcItem;
	COLORREF crOldTextColor = dc.SetTextColor(m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	const int iMarginX = 4;
	cur_rec.right = cur_rec.left - iMarginX*2;
	cur_rec.left += iMarginX;

	/*
	int iIconDrawWidth = theApp.GetSmallSytemIconSize().cx + 3;
	*/
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			/*
			UINT uDTFlags = DLC_DT_TEXT;
			*/
			cur_rec.right += GetColumnWidth(iColumn);
			switch(iColumn){
				case 0:{
					CFont boldFont;
					CFont* pOldNormalFont = NULL;

					if(bGlowing){
						LOGFONT boldLogFont;
						GetFont()->GetLogFont(&boldLogFont);
						boldLogFont.lfWeight = FW_BOLD;
						boldFont.CreateFontIndirect(&boldLogFont);
						pOldNormalFont = dc.SelectObject(&boldFont);
					}

					CRect tmpRC;
					int txtHeight;
					int rectWidth;
					tmpRC.CopyRect(&cur_rec);
					tmpRC.top+=iMarginX;
					txtHeight = dc->DrawText(skinItem->name, skinItem->name.GetLength(),&tmpRC,DT_LEFT|DT_SINGLELINE|DT_TOP|DT_NOPREFIX|DT_END_ELLIPSIS);
					cur_rec.top += txtHeight + iMarginX*2;

					rectWidth = cur_rec.right - cur_rec.left;
					if(rectWidth > 504)
						rectWidth = 504;

					if(!skinItem->image || (skinItem->image->GetWidth() != (DWORD)rectWidth)){
						CxImage *xImage=NULL;
						xImage = GetSkinPreview(*skinItem, cur_rec.right - cur_rec.left);
						if(xImage){
							delete skinItem->image;
							skinItem->image = xImage;
						}
					}
					if(skinItem->image){
						skinItem->image->Draw(dc,cur_rec.left, cur_rec.top + (((cur_rec.bottom - cur_rec.top) / 2) - (skinItem->image->GetHeight() / 2)));
					}

					if(bGlowing)
						dc.SelectObject(pOldNormalFont);

					break;
				}
			}
		}
	}
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc->FrameRect(&outline_rec, &CBrush(m_crWindow));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if (lpDrawItemStruct->itemID > 0 && GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED))
			outline_rec.top--;

		if (lpDrawItemStruct->itemID + 1 < (UINT)GetItemCount() && GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED))
			outline_rec.bottom++;

		if(bCtrlFocused)
			dc->FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc->FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

CxImage* CSkinsListCtrl::GetToolBarPreview(CString toolbarFile, int width /*= -1*/)
{
	CDC dcMem, dcMem2;
	CDC* pDC = CWnd::GetDesktopWindow()->GetDC();
	/*
	BOOL bResult = FALSE;
	*/
	int previewHeight = 40;
	int previewWidth;

	previewWidth = 504;

	if(width>=previewWidth)
		width = -1;

	CxImage xImg, *xImg2;
	xImg2 = new CxImage();

	if (xImg2 && dcMem.CreateCompatibleDC(pDC)
		&& dcMem2.CreateCompatibleDC(pDC))
	{
		bool bImageOK = false;
		// Modified by MoNKi [MoNKi: -emulEspaña Skin-]
		/*
		bImageOK = xImg.Load(toolbarFile);
		*/
//		if(toolbarFile == EMULESPANA_DEFAULT_SKIN)
//			bImageOK = xImg.LoadResource(FindResource(NULL,MAKEINTRESOURCE(IDR_EE_TOOLBAR_SKIN_PNG),_T("PNG")), CXIMAGE_FORMAT_PNG);
//		else
			bImageOK = xImg.Load(toolbarFile);
		// End -emulEspaña Skin-
		if(bImageOK){
			CBitmap bmMem, bmMem2;
			CPen penHighLight;
			CPen penShadow;
			CPen *pOldPen;

			previewHeight = min(xImg.GetHeight() + 8, 40);

			RGBQUAD maskColor;
			maskColor.rgbBlue = maskColor.rgbRed = 255;
			maskColor.rgbGreen = 0;
			xImg.SetTransColor(maskColor);
			xImg.SetTransIndex(0);

			penHighLight.CreatePen(PS_SOLID,1, GetSysColor(COLOR_3DLIGHT));
			penShadow.CreatePen(PS_SOLID,1, GetSysColor(COLOR_3DSHADOW));

			if (bmMem.CreateCompatibleBitmap(pDC, previewWidth, previewHeight)
				&& bmMem2.CreateCompatibleBitmap(pDC, previewWidth, previewHeight)){
				CBitmap* pOldBM = dcMem.SelectObject(&bmMem);
				CBitmap* pOldBM2 = dcMem2.SelectObject(&bmMem2);
				CRect rc;
			   
				//Fill dcMem with button's color and draws the preview
				dcMem.FillSolidRect(0, 0, previewWidth, previewHeight, GetSysColor(COLOR_3DFACE));
				dcMem2.FillSolidRect(0, 0, previewWidth, previewHeight, RGB(255,255,255));

				pOldPen = dcMem.SelectObject(&penShadow);
				dcMem.MoveTo(0,0);
				dcMem.LineTo(previewWidth-1, 0);
				dcMem.LineTo(previewWidth-1, previewHeight-1);
				dcMem.LineTo(0, previewHeight-1);
				dcMem.LineTo(0, 0);
				dcMem.SelectObject(pOldPen);
				
				rc.left=0; rc.top=4; rc.right=xImg.GetHeight(); rc.bottom=xImg.GetHeight()+4;

				for(int i = 0; i<12; i++){
					int j, k;
					if (i==0){
						j = 1;
						k = 0;
					}
					else if (i<9){
						j = 2;
						k = 2;
					}
					else{
						j = 3;
						k = 2;
					}
					rc.MoveToX(xImg.GetHeight()*i + 8*(i+j));
					xImg.Draw(dcMem, 8*(i+j) - (xImg.GetHeight()*k) , 4, -1, -1, &rc);
				}


				int offset1, offset2, offset3, offset4, offset5;
				offset1 = xImg.GetHeight() + 16;
				offset2 = xImg.GetHeight() + 17;
				offset3 = xImg.GetHeight() + 4;
				offset4 = xImg.GetHeight()*9 + 88;
				offset5 = xImg.GetHeight()*9 + 89;

				pOldPen = dcMem.SelectObject(&penShadow);
				dcMem.MoveTo(offset1,4);
				dcMem.LineTo(offset1, offset3);
				dcMem.SelectObject(&penHighLight);
				dcMem.MoveTo(offset2,4);
				dcMem.LineTo(offset2, offset3);
				
				dcMem.SelectObject(&penShadow);
				dcMem.MoveTo(offset4 ,4);
				dcMem.LineTo(offset4,offset3);
				dcMem.SelectObject(&penHighLight);
				dcMem.MoveTo(offset5,4);
				dcMem.LineTo(offset5,offset3);

				dcMem.SelectObject(pOldPen);

				if(width >= 0){
					pOldPen = dcMem.SelectObject(&penShadow);

					CPoint pts[9];
					pts[0].x = width - 10;
					pts[0].y = -1;

					pts[1].x = width-1;
					pts[1].y = 6;

					pts[2].x = width-6;
					pts[2].y = 12;

					pts[3].x = width-1;
					pts[3].y = 20;

					pts[4].x = width - 8;
					pts[4].y = 32;

					pts[5].x = width - 1;
					pts[5].y = 35;

					pts[6].x = width - 4;
					pts[6].y = 40;

					pts[7].x = 504;
					pts[7].y = 40;

					pts[8].x = 504;
					pts[8].y = -1;

					dcMem.Polygon(pts, 9);
					dcMem.SelectObject(pOldPen);

					for(int i=0;i<9;i++){
						pts[i].Offset(1,0);
					}
					CBrush blackBrush(RGB(0,0,0));
					CBrush* pOldBrush;
					pOldBrush = dcMem2.SelectObject(&blackBrush);
					dcMem2.Polygon(pts, 9);
					dcMem2.SelectObject(pOldBrush);
				}

				if(xImg2->CreateFromHBITMAP(bmMem)){
					CxImage xImg3;
					if(xImg3.CreateFromHBITMAP(bmMem2)){
						xImg3.GrayScale();
						xImg2->AlphaSet(xImg3);
						xImg2->AlphaPaletteEnable();
					}
					if(width >= 0)
						xImg2->Crop(0,0,width, previewHeight);
				}
				dcMem.SelectObject(pOldBM);
				dcMem2.SelectObject(pOldBM2);
			}
		}
	}

	CWnd::GetDesktopWindow()->ReleaseDC(pDC);

	if(xImg2 && xImg2->IsValid()){
		return xImg2;
	}
	else{
		if(xImg2)
			delete xImg2;
		return NULL;
	}
}

CxImage* CSkinsListCtrl::GetIniPreview(CString iniFile, int width /*= -1*/){

	CImageList imgList;

	imgList.Create(32, 32, ILC_COLOR32 | ILC_MASK, 0, 1);
	imgList.Add(CTempIconLoader(_T("CONNECT"), 32, 32, LR_DEFAULTCOLOR, iniFile));
	imgList.Add(CTempIconLoader(_T("KADEMLIA"), 32, 32, LR_DEFAULTCOLOR, iniFile));
	imgList.Add(CTempIconLoader(_T("SERVER"), 32, 32, LR_DEFAULTCOLOR, iniFile));
	imgList.Add(CTempIconLoader(_T("TRANSFER"), 32, 32, LR_DEFAULTCOLOR, iniFile));
	imgList.Add(CTempIconLoader(_T("SEARCH"), 32, 32, LR_DEFAULTCOLOR, iniFile));
	imgList.Add(CTempIconLoader(_T("SharedFiles"), 32, 32, LR_DEFAULTCOLOR, iniFile));
	imgList.Add(CTempIconLoader(_T("MESSAGES"), 32, 32, LR_DEFAULTCOLOR, iniFile));
	imgList.Add(CTempIconLoader(_T("IRC"), 32, 32, LR_DEFAULTCOLOR, iniFile));
	imgList.Add(CTempIconLoader(_T("STATISTICS"), 32, 32, LR_DEFAULTCOLOR, iniFile));
	imgList.Add(CTempIconLoader(_T("PREFERENCES"), 32, 32, LR_DEFAULTCOLOR, iniFile));
	imgList.Add(CTempIconLoader(_T("TOOLS"), 32, 32, LR_DEFAULTCOLOR, iniFile));
	imgList.Add(CTempIconLoader(_T("HELP"), 32, 32, LR_DEFAULTCOLOR, iniFile));
	
	CDC dcMem, dcMem2;
	CDC* pDC = CWnd::GetDesktopWindow()->GetDC();
	/*
	BOOL bResult = FALSE;
	*/
	int previewHeight = 40;
	int previewWidth;

	previewWidth = 504;

	if(width>=previewWidth)
		width = -1;

	CxImage *xImg2;
	xImg2 = new CxImage();

	if (xImg2 && dcMem.CreateCompatibleDC(pDC)
		&& dcMem2.CreateCompatibleDC(pDC))
	{
		CBitmap bmMem, bmMem2;
		CPen penHighLight;
		CPen penShadow;
		CPen *pOldPen;

		penHighLight.CreatePen(PS_SOLID,1, GetSysColor(COLOR_3DLIGHT));
		penShadow.CreatePen(PS_SOLID,1, GetSysColor(COLOR_3DSHADOW));

		if (bmMem.CreateCompatibleBitmap(pDC, previewWidth, previewHeight)
			&& bmMem2.CreateCompatibleBitmap(pDC, previewWidth, previewHeight)){
			CBitmap* pOldBM = dcMem.SelectObject(&bmMem);
			CBitmap* pOldBM2 = dcMem2.SelectObject(&bmMem2);
			CRect rc;
			
			//Fill dcMem with button's color and draws the preview
			dcMem.FillSolidRect(0, 0, previewWidth, previewHeight, GetSysColor(COLOR_3DFACE));
			dcMem2.FillSolidRect(0, 0, previewWidth, previewHeight, RGB(255,255,255));

			pOldPen = dcMem.SelectObject(&penShadow);
			dcMem.MoveTo(0,0);
			dcMem.LineTo(previewWidth-1, 0);
			dcMem.LineTo(previewWidth-1, previewHeight-1);
			dcMem.LineTo(0, previewHeight-1);
			dcMem.LineTo(0, 0);
			dcMem.SelectObject(pOldPen);
			
			rc.left=0; rc.top=4; rc.right=32; rc.bottom=36;

			for(int i = 0; i<12; i++){
				int j;
				if (i==0){
					j = 1;
				}
				else if (i<9){
					j = 2;
				}
				else{
					j = 3;
				}
				rc.MoveToX(32*i + 8*(i+j));
				imgList.DrawIndirect(&dcMem, i, CPoint(8*(i+j) + (32*i),4), CSize(32, 32), CPoint(0, 0));

			}


			pOldPen = dcMem.SelectObject(&penShadow);
			dcMem.MoveTo(48,4);
			dcMem.LineTo(48,36);
			dcMem.SelectObject(&penHighLight);
			dcMem.MoveTo(49,4);
			dcMem.LineTo(49,36);
			
			dcMem.SelectObject(&penShadow);
			dcMem.MoveTo(376,4);
			dcMem.LineTo(376,36);
			dcMem.SelectObject(&penHighLight);
			dcMem.MoveTo(377,4);
			dcMem.LineTo(377,36);

			dcMem.SelectObject(pOldPen);

			if(width >= 0){
				pOldPen = dcMem.SelectObject(&penShadow);

				CPoint pts[9];
				pts[0].x = width - 10;
				pts[0].y = -1;

				pts[1].x = width-1;
				pts[1].y = 6;

				pts[2].x = width-6;
				pts[2].y = 12;

				pts[3].x = width-1;
				pts[3].y = 20;

				pts[4].x = width - 8;
				pts[4].y = 32;

				pts[5].x = width - 1;
				pts[5].y = 35;

				pts[6].x = width - 4;
				pts[6].y = 40;

				pts[7].x = 504;
				pts[7].y = 40;

				pts[8].x = 504;
				pts[8].y = -1;

				dcMem.Polygon(pts, 9);
				dcMem.SelectObject(pOldPen);

				for(int i=0;i<9;i++){
					pts[i].Offset(1,0);
				}
				CBrush blackBrush(RGB(0,0,0));
				CBrush* pOldBrush;
				pOldBrush = dcMem2.SelectObject(&blackBrush);
				dcMem2.Polygon(pts, 9);
				dcMem2.SelectObject(pOldBrush);
			}

			if(xImg2->CreateFromHBITMAP(bmMem)){
				CxImage xImg3;
				if(xImg3.CreateFromHBITMAP(bmMem2)){
					xImg3.GrayScale();
					xImg2->AlphaSet(xImg3);
					xImg2->AlphaPaletteEnable();
				}
				if(width >= 0)
					xImg2->Crop(0,0,width, previewHeight);
			}
			dcMem.SelectObject(pOldBM);
			dcMem2.SelectObject(pOldBM2);
		}
	}

	CWnd::GetDesktopWindow()->ReleaseDC(pDC);

	if(xImg2 && xImg2->IsValid()){
		return xImg2;
	}
	else{
		if(xImg2)
			delete xImg2;
		return NULL;
	}
}

CxImage* CSkinsListCtrl::GetSkinPreview(SKIN_ITEM &item, int width /*= -1*/){
	if(item.bToolBar && item.iniFile != EMULE_DEFAULT_SKIN)
		return GetToolBarPreview(item.toolbarFile, width);
	else 
		return GetIniPreview(item.iniFile, width);
}

void CSkinsListCtrl::LoadToolBars(CString path){
	RemoveItems();

	m_iMode = MODE_TOOLBARS;
	int curItem=-1;
	
	CString currentTbLower = thePrefs.GetToolbarBitmapSettings();
	currentTbLower.MakeLower();

	LVCOLUMN col;
	CString colText;
	colText = GetResString(IDS_TOOLBARSKINS);
	col.mask = LVCF_TEXT;
	col.pszText = colText.GetBuffer();
	SetColumn(0,&col);
	colText.ReleaseBuffer();

	SKIN_ITEM *currentSkin = NULL;
	SKIN_ITEM *skinItem = NULL;
	skinItem = new SKIN_ITEM;
	skinItem->name = GetResString(IDS_DEFAULT);
	skinItem->image = NULL;
	skinItem->iniFile = EMULE_DEFAULT_SKIN;
	skinItem->bToolBar = true;
	m_skinsList.AddTail(skinItem);
	curItem = InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,0,(LPCTSTR)skinItem->name,0,0,1,(LPARAM)skinItem);
	if(currentTbLower.IsEmpty()){
		m_iCurrent = curItem;
		currentSkin = skinItem;

		SetItemState(m_iCurrent,LVIS_GLOW,LVIS_GLOW);
		SetItemState(m_iCurrent, LVIS_SELECTED, LVIS_SELECTED);
		SetItemState(m_iCurrent, LVIS_FOCUSED, LVIS_FOCUSED);
	}

	if (!path.IsEmpty())
	{
		for (int f = 0; f < _countof(_apszTBFiles); f++)
		{
			bool bFinished = false;
			WIN32_FIND_DATA FileData;
			HANDLE hSearch = FindFirstFile(path + CString(_T("\\")) + _apszTBFiles[f], &FileData);
			if (hSearch == INVALID_HANDLE_VALUE)
				bFinished = true;
			while(!bFinished)
			{
				CString bitmapFileName = FileData.cFileName;
				CString bitmapBaseName;
				int iExt = CString(bitmapFileName).MakeLower().Find(CString(EMULTB_BASEEXT).MakeLower());
				if (iExt > 0)
					bitmapBaseName = bitmapFileName.Left(iExt - 1);
				else
					bitmapBaseName = bitmapFileName;

				skinItem = new SKIN_ITEM;
				skinItem->name = bitmapBaseName;
				skinItem->toolbarFile = path + CString(_T("\\")) + bitmapFileName;
				skinItem->image = NULL;
				skinItem->bToolBar = true;
				m_skinsList.AddTail(skinItem);
				curItem = InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,m_skinsList.GetCount()-1,(LPCTSTR)skinItem->name,0,0,1,(LPARAM)skinItem);
				if(CString(skinItem->toolbarFile).MakeLower() == currentTbLower){
					m_iCurrent = curItem;
					currentSkin = skinItem;

					SetItemState(m_iCurrent,LVIS_GLOW,LVIS_GLOW);
					SetItemState(m_iCurrent, LVIS_SELECTED, LVIS_SELECTED);
					SetItemState(m_iCurrent, LVIS_FOCUSED, LVIS_FOCUSED);
				}

				if (!FindNextFile(hSearch, &FileData))
					bFinished = true;
			}
			FindClose(hSearch);
		}
	}

	if(m_iCurrent == -1){
		CString skinTitle;
		CString curSkin = thePrefs.GetToolbarBitmapSettings();
		int charpos = currentTbLower.Find(CString(EMULTB_BASEEXT).MakeLower());
		if (charpos > 0)
			skinTitle = curSkin.Left(charpos - 1);
		else
			skinTitle = curSkin;

		charpos = skinTitle.ReverseFind(_T('\\'));
		if (charpos != -1)
			skinTitle = skinTitle.Right(skinTitle.GetLength() - charpos - 1);

		skinItem = new SKIN_ITEM;
		skinItem->name = skinTitle;
		skinItem->image = NULL;
		skinItem->toolbarFile = curSkin;
		skinItem->bToolBar = true;
		m_skinsList.AddTail(skinItem);
		curItem = InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,m_skinsList.GetCount()-1,(LPCTSTR)skinItem->name,0,0,1,(LPARAM)skinItem);

		m_iCurrent = curItem;
		currentSkin = skinItem;

		SetItemState(m_iCurrent,LVIS_GLOW,LVIS_GLOW);
		SetItemState(m_iCurrent, LVIS_SELECTED, LVIS_SELECTED);
		SetItemState(m_iCurrent, LVIS_FOCUSED, LVIS_FOCUSED);
	}

	if(m_iCurrent != -1){
		//The current item index can be changed due to new insertions
		LVFINDINFO findInfo;
		findInfo.lParam = (LPARAM)currentSkin;
		findInfo.flags = LVFI_PARAM;
		m_iCurrent = FindItem(&findInfo);
		EnsureVisible(m_iCurrent, false);
	}
}

void CSkinsListCtrl::LoadSkins(CString path){
	RemoveItems();

	int curItem = -1;

	m_iMode = MODE_SKINS;

	CString currentSkinLower = thePrefs.GetSkinProfile();
	currentSkinLower.MakeLower();

	LVCOLUMN col;
	CString colText;
	colText = GetResString(IDS_SKIN_PROF);
	col.mask = LVCF_TEXT;
	col.pszText = colText.GetBuffer();
	SetColumn(0,&col);
	colText.ReleaseBuffer();

	SKIN_ITEM *currentSkin = NULL;
	SKIN_ITEM *skinItem = NULL;
	skinItem = new SKIN_ITEM;
	skinItem->name = GetResString(IDS_DEFAULT);
	skinItem->image = NULL;
	skinItem->iniFile = EMULE_DEFAULT_SKIN;
	skinItem->bToolBar = false;
	m_skinsList.AddTail(skinItem);
	curItem = InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,0,(LPCTSTR)skinItem->name,0,0,1,(LPARAM)skinItem);
	if(currentSkinLower.IsEmpty()){
		m_iCurrent = curItem;
		currentSkin = skinItem;

		SetItemState(m_iCurrent,LVIS_GLOW,LVIS_GLOW);
		SetItemState(m_iCurrent, LVIS_SELECTED, LVIS_SELECTED);
		SetItemState(m_iCurrent, LVIS_FOCUSED, LVIS_FOCUSED);
	}

	if (!path.IsEmpty())
	{
		for (int f = 0; f < _countof(_apszSkinFiles); f++)
		{
			bool bFinished = false;
			WIN32_FIND_DATA FileData;
			HANDLE hSearch = FindFirstFile(path + CString(_T("\\")) + _apszSkinFiles[f], &FileData);
			if (hSearch == INVALID_HANDLE_VALUE)
				bFinished = true;
			while(!bFinished)
			{
				CString skinFileName = FileData.cFileName;
				CString skinBaseName;
				int iExt = CString(skinFileName).MakeLower().Find(CString(EMULSKIN_BASEEXT).MakeLower());
				if (iExt > 0)
					skinBaseName = skinFileName.Left(iExt - 1);
				else
					skinBaseName = skinFileName;

				skinItem = new SKIN_ITEM;
				skinItem->name = skinBaseName;
				skinItem->iniFile = path + CString(_T("\\")) + skinFileName;
				skinItem->image = NULL;
				skinItem->bToolBar = false;
				m_skinsList.AddTail(skinItem);
				curItem = InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,m_skinsList.GetCount()-1,(LPCTSTR)skinItem->name,0,0,1,(LPARAM)skinItem);
				if(CString(skinItem->iniFile).MakeLower() == currentSkinLower){
					m_iCurrent = curItem;
					currentSkin = skinItem;
					SetItemState(m_iCurrent,LVIS_GLOW,LVIS_GLOW);
					SetItemState(m_iCurrent, LVIS_SELECTED, LVIS_SELECTED);
					SetItemState(m_iCurrent, LVIS_FOCUSED, LVIS_FOCUSED);
				}

				if (!FindNextFile(hSearch, &FileData))
					bFinished = true;
			}
			FindClose(hSearch);
		}
	}

	if(m_iCurrent == -1){
		CString skinTitle;
		CString curSkin = thePrefs.GetSkinProfile();
		int charpos = currentSkinLower.Find(CString(EMULSKIN_BASEEXT).MakeLower());
		if (charpos > 0)
			skinTitle = curSkin.Left(charpos - 1);
		else
			skinTitle = curSkin;

		charpos = skinTitle.ReverseFind(_T('\\'));
		if (charpos != -1)
			skinTitle = skinTitle.Right(skinTitle.GetLength() - charpos - 1);

		skinItem = new SKIN_ITEM;
		skinItem->name = skinTitle;
		skinItem->image = NULL;
		skinItem->iniFile = curSkin;
		skinItem->bToolBar = false;
		m_skinsList.AddTail(skinItem);
		curItem = InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,m_skinsList.GetCount()-1,(LPCTSTR)skinItem->name,0,0,1,(LPARAM)skinItem);

		m_iCurrent = curItem;
		currentSkin = skinItem;

		SetItemState(m_iCurrent,LVIS_GLOW,LVIS_GLOW);
		SetItemState(m_iCurrent, LVIS_SELECTED, LVIS_SELECTED);
		SetItemState(m_iCurrent, LVIS_FOCUSED, LVIS_FOCUSED);
	}

	if(m_iCurrent != -1){
		//The current item index can be changed due to new insertions
		LVFINDINFO findInfo;
		findInfo.lParam = (LPARAM)currentSkin;
		findInfo.flags = LVFI_PARAM;
		m_iCurrent = FindItem(&findInfo);
		EnsureVisible(m_iCurrent, false);
	}
}

void CSkinsListCtrl::RemoveItems(){
	POSITION pos = m_skinsList.GetHeadPosition();
	while(pos){
		SKIN_ITEM *item;
		item = m_skinsList.GetNext(pos);
		delete item->image;
		delete item;
	}
	m_skinsList.RemoveAll();
	
	if( theApp.IsRunning() && m_hWnd != NULL )
		DeleteAllItems();

	m_iCurrent = -1;
	m_iMode = MODE_NONE;
}

void CSkinsListCtrl::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	SelectCurrentSkin();
	*pResult = 0;
}

void CSkinsListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();

	POSITION pos = m_skinsList.GetHeadPosition();
	while(pos){
		SKIN_ITEM *item;
		item = m_skinsList.GetNext(pos);
		delete item->image;
		item->image = NULL;
	}
}

void CSkinsListCtrl::Localize(){
	LVCOLUMN col;
	CString colText;
	BOOL bChangeDefault = false;
    
	switch(m_iMode){
		case MODE_TOOLBARS:
			colText = GetResString(IDS_TOOLBARSKINS);
			bChangeDefault = true;
			break;
		case MODE_SKINS:
			colText = GetResString(IDS_SKIN_PROF);
			bChangeDefault = true;
			break;
		default:
			colText = "Skins";
			bChangeDefault = false;
			break;
	}

	col.mask = LVCF_TEXT;
	col.pszText = colText.GetBuffer();
	SetColumn(0,&col);
	colText.ReleaseBuffer();

	if(bChangeDefault && GetItemCount() > 0 ){
		SKIN_ITEM *itemData;
		itemData = (SKIN_ITEM *)GetItemData(0);
		itemData->name = GetResString(IDS_DEFAULT);
		SetItemText(0,0,GetResString(IDS_DEFAULT));
	}
}

CString CSkinsListCtrl::GetSelectedSkin(){
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		SKIN_ITEM *item;
		item = (SKIN_ITEM *)GetItemData(iSel);
		if(item->bToolBar){
			return item->toolbarFile;
		}
		else{ 
			return item->iniFile;
		}
	}	
	return CString(_T(""));
}

void CSkinsListCtrl::SelectCurrentSkin()
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		SKIN_ITEM *item;
		item = (SKIN_ITEM *)GetItemData(iSel);
		if(item->bToolBar){
			thePrefs.SetToolbarBitmapSettings(item->toolbarFile);
			theApp.emuledlg->toolbar->ChangeToolbarBitmap(item->toolbarFile, true);
		}
		else{ 
			theApp.ApplySkin(item->iniFile);

			// the parent should send WM_SYSCOLORCHANGE to his childs
			GetParent()->SendMessage(WM_SYSCOLORCHANGE);
		}
	
		if(m_iCurrent != -1)
			SetItemState(m_iCurrent,0,LVIS_GLOW);

		SetItemState(iSel,LVIS_GLOW,LVIS_GLOW);
		m_iCurrent = iSel;
	}
}

void CSkinsListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CTitleMenu skinMenu;
	skinMenu.CreatePopupMenu();
	skinMenu.AppendMenu(MF_STRING | (GetSelectedCount()>0 ? MF_ENABLED : MF_GRAYED),CSkinsListCtrl::MP_APPLY_CURRENT_SKIN,GetResString(IDS_MOD_APLY_SKIN));
	skinMenu.AppendMenu(MF_STRING,CSkinsListCtrl::MP_SELECT_SKIN_FILE,GetResString(IDS_MOD_BROWSE_SKIN));
	skinMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	skinMenu.AppendMenu(MF_STRING,CSkinsListCtrl::MP_GET_MORE_SKINS,GetResString(IDS_MOD_GET_SKINS));
	
	skinMenu.SetDefaultItem(CSkinsListCtrl::MP_APPLY_CURRENT_SKIN, false);
	
	skinMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);

	VERIFY( skinMenu.DestroyMenu() );

}

BOOL CSkinsListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	UINT selectedCount = this->GetSelectedCount(); 
	/*
	int iSel = GetSelectionMark();
	*/

	if (selectedCount>0){
		if(wParam == CSkinsListCtrl::MP_APPLY_CURRENT_SKIN)
			SelectCurrentSkin();
	}

	if(wParam == CSkinsListCtrl::MP_SELECT_SKIN_FILE){
		CString strFilter;
		if(m_iMode == CSkinsListCtrl::MODE_TOOLBARS || m_iMode == CSkinsListCtrl::MODE_NONE){
			strFilter = _T("eMule Toolbar Bitmap Files (");
			for (int f = 0; f < _countof(_apszTBFiles); f++){
				if (f > 0)
					strFilter += _T(';');
				strFilter += _apszTBFiles[f];
			}
			strFilter += _T(")|");
			for (int f = 0; f < _countof(_apszTBFiles); f++){
				if (f > 0)
					strFilter += _T(';');
				strFilter += _apszTBFiles[f];
			}
		}
		if(m_iMode == CSkinsListCtrl::MODE_SKINS || m_iMode == CSkinsListCtrl::MODE_NONE){
			strFilter += _T("eMule Skin Files (");
			for (int f = 0; f < _countof(_apszSkinFiles); f++){
				if (f > 0)
					strFilter += _T(';');
				strFilter += _apszSkinFiles[f];
			}
			strFilter += _T(")|");
			for (int f = 0; f < _countof(_apszSkinFiles); f++){
				if (f > 0)
					strFilter += _T(';');
				strFilter += _apszSkinFiles[f];
			}
		}
		strFilter += _T("||");

		CFileDialog dialog(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST, strFilter, NULL, 0);
		if (IDOK == dialog.DoModal()){
			AddFile(dialog.GetPathName());
		}
	}
	else if(wParam == CSkinsListCtrl::MP_GET_MORE_SKINS){
		ShellExecute(NULL, _T("open"), _T("http://www.kademliaskins.net"),NULL, NULL, SW_SHOW);
	}
	
	return true;
}

bool CSkinsListCtrl::AddFile(CString file){
	bool ret = false;
	long itemPos = 0;
	int curItem = -1;

	CString lowerFile(file);
	lowerFile.MakeLower();

	//Is the file in the list?
	bool found = false;
	POSITION pos = m_skinsList.GetHeadPosition();
	while(pos && !found){
		SKIN_ITEM *item;
		item = m_skinsList.GetNext(pos);
		if(CString(item->iniFile).MakeLower() == lowerFile || CString(item->toolbarFile).MakeLower() == lowerFile)
			found = true;
		else
			itemPos++;
	}

	if(!found){
		found = false;
		CString extension, fileext, base;
		int arrSize = 0;
		bool toolbar = false;

		if(m_iMode == CSkinsListCtrl::MODE_TOOLBARS){
			arrSize = _countof(_apszTBFiles);
			toolbar = true;
			base = EMULTB_BASEEXT;
		}
		else if(m_iMode == CSkinsListCtrl::MODE_SKINS){
			arrSize = _countof(_apszSkinFiles);
			toolbar = false;
			base = EMULSKIN_BASEEXT;
		}			

		if(m_iMode == CSkinsListCtrl::MODE_TOOLBARS || m_iMode == CSkinsListCtrl::MODE_SKINS){
			base.MakeLower();
			for (int i = 0; i < arrSize; i++){
				if(toolbar)
					extension = _apszTBFiles[i];
				else
					extension = _apszSkinFiles[i];
				extension = extension.Right(extension.GetLength()-2).MakeLower();
				fileext = file.Right(extension.GetLength()).MakeLower();
				if(extension == fileext)
					found = true;
			}
			if(found){
				CString skinTitle;
				int charpos = lowerFile.Find(base);
				if (charpos > 0)
					skinTitle = file.Left(charpos - 1);
				else
					skinTitle = file;

				charpos = skinTitle.ReverseFind(_T('\\'));
				if (charpos != -1)
					skinTitle = skinTitle.Right(skinTitle.GetLength() - charpos - 1);

				SKIN_ITEM *skinItem;
				skinItem = new SKIN_ITEM;
				skinItem->name = skinTitle;
				skinItem->image = NULL;
				skinItem->bToolBar = toolbar;
				if(toolbar)
					skinItem->toolbarFile = file;
				else
					skinItem->iniFile = file;

				m_skinsList.AddTail(skinItem);
				curItem = InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,m_skinsList.GetCount()-1,(LPCTSTR)skinItem->name,0,0,1,(LPARAM)skinItem);

				m_iCurrent = curItem;

				SetItemState(m_iCurrent, LVIS_SELECTED, LVIS_SELECTED);
				SetItemState(m_iCurrent, LVIS_FOCUSED, LVIS_FOCUSED);

				EnsureVisible(m_iCurrent, false);

				ret = true;
			}
		}
		else{
			//CSkinsListCtrl::MODE_NONE
			// TODO: Not implemented, because for now we not use it
			ret = false;
		}
	}
	else{
		SetItemState(itemPos, LVIS_SELECTED, LVIS_SELECTED);
		SetItemState(itemPos, LVIS_FOCUSED, LVIS_FOCUSED);
		EnsureVisible(itemPos, false);
	}

	return ret;
}

void CSkinsListCtrl::OnColumnClick( NMHDR* /*pNMHDR*/, LRESULT* pResult){
	/*
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*/

	// Sort table
	m_bOrderAscending = !m_bOrderAscending;

	SetSortArrow(0, m_bOrderAscending);
	SortItems(SortProc, 1 + (m_bOrderAscending ? 0:20));

	*pResult = 0;
}

int CSkinsListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	SKIN_ITEM* item1 = (SKIN_ITEM*)lParam1;
	SKIN_ITEM* item2 = (SKIN_ITEM*)lParam2;	
	switch(lParamSort){
		case 1: //Name asc
			if(item1->iniFile == EMULE_DEFAULT_SKIN)
				return -1;
			else if(item2->iniFile == EMULE_DEFAULT_SKIN)
				return 1;
			else
				return _tcsicmp(item1->name,item2->name);
		case 21: //Name desc
			if(item1->iniFile == EMULE_DEFAULT_SKIN)
				return -1;
			else if(item2->iniFile == EMULE_DEFAULT_SKIN)
				return 1;
			else
				return _tcsicmp(item2->name,item1->name);
		default: 
			return 0;
	}
}
