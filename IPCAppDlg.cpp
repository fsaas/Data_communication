// 
// 학번 : 201007142
// 이름 : 양태훈
//
// IPCAppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ipc.h"
#include "IPCAppDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIPCAppDlg dialog

CIPCAppDlg::CIPCAppDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIPCAppDlg::IDD, pParent), 
	CBaseLayer( "ChatDlg" ),
	m_bSendReady(FALSE)
{
	//{{AFX_DATA_INIT(CIPCAppDlg)
	m_unSrcEnetAddr = "1";
	m_unDstEnetAddr = "2";

	m_stMessage = _T("");
	m_filePath = _T("");

	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_LayerMgr.AddLayer( this ) ;	
	m_LayerMgr.AddLayer( new CNILayer( "NI" ) ) ;
	m_LayerMgr.AddLayer( new CEthernetLayer( "Ethernet" ) ) ;
	m_LayerMgr.AddLayer( new CIPLayer( "IP" ) );
	m_LayerMgr.AddLayer( new CTCPLayer( "TCP" ) );
	m_LayerMgr.AddLayer( new CChatAppLayer( "ChatApp" ) ) ;
	m_LayerMgr.AddLayer( new CFileAppLayer( "FileApp" ) ) ;

	//////////////////////// fill the blank ///////////////////////////////	


	///////////////////////////////////////////////////////////////////////

	m_ChatApp = (CChatAppLayer*)mp_UnderLayer ;
	m_TCP = (CTCPLayer *)m_LayerMgr.GetLayer("TCP");
	m_IP = (CIPLayer *)m_LayerMgr.GetLayer("IP");
	m_ETH = (CNILayer *)m_LayerMgr.GetLayer("Ethernet");
	m_NI = (CNILayer *)m_LayerMgr.GetLayer("NI");
}

void CIPCAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIPCAppDlg)
	DDX_Control(pDX, IDC_LIST_CHAT, m_ListChat);
	DDX_Text(pDX, IDC_EDIT_DST, m_unDstEnetAddr);
	DDX_Text(pDX, IDC_EDIT_SRC, m_unSrcEnetAddr);
	DDX_Text(pDX, IDC_EDIT_MSG, m_stMessage);

	DDX_Control(pDX, IDC_EDIT_SRCIP, m_unSrcIPAddr);
	DDX_Control(pDX, IDC_EDIT_DSTIP, m_unDstIPAddr);
	DDX_Text(pDX, IDC_EDIT_FilePath, m_filePath);

	DDX_Control(pDX,IDC_PROGRESS, m_ProgressCtrl);
	DDX_Control(pDX, IDC_COMBO_ENETADDR, m_ComboEnetName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIPCAppDlg, CDialog)
	//{{AFX_MSG_MAP(CIPCAppDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SEND, OnSendMessage)
	ON_BN_CLICKED(IDC_BUTTON_ADDR, OnButtonAddrSet)
	ON_BN_CLICKED(IDC_CHECK_TOALL, OnCheckBroadcast)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDC_BUTTON_FILE, OnAddFile)
	ON_BN_CLICKED(IDC_BUTTON_FILESEND, OnSendFile)

	ON_CBN_SELCHANGE(IDC_COMBO_ENETADDR, OnComboEnetAddr)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIPCAppDlg message handlers

BOOL CIPCAppDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	SetRegstryMessage( ) ;
	SetDlgState(IPC_INITIALIZING);
	SetDlgState(CFT_COMBO_SET);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIPCAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ( nID == SC_CLOSE )
	{
		if ( MessageBox( "Are you sure ?", 
			"Question", 
			MB_YESNO | MB_ICONQUESTION ) 
			== IDNO )
			return ;
		else EndofProcess( ) ;
	}

	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIPCAppDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CIPCAppDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CIPCAppDlg::OnSendMessage() 
{
	UpdateData( TRUE ) ;

	if ( !m_stMessage.IsEmpty() )
	{
		SetTimer(1,3000,NULL);
	
		SendData( ) ;

		m_stMessage = "" ;

		(CEdit*) GetDlgItem( IDC_EDIT_MSG )->SetFocus( ) ;
	}
	UpdateData( FALSE ) ;
}

void CIPCAppDlg::OnButtonAddrSet() 
{
	UpdateData( TRUE ) ;
	unsigned char src_ip[4];
	unsigned char dst_ip[4];

	if ( !m_unDstEnetAddr || 
		!m_unSrcEnetAddr  )
	{
		MessageBox( "주소를 설정 오류발생", 
			"경고", 
			MB_OK | MB_ICONSTOP ) ;

		return ;
	}

	if ( m_bSendReady ){
		SetDlgState( IPC_ADDR_RESET ) ;
		SetDlgState( IPC_INITIALIZING ) ;
	}
	else{
		m_unSrcIPAddr.GetAddress(src_ip[0],src_ip[1],src_ip[2],src_ip[3]);
		m_unDstIPAddr.GetAddress(dst_ip[0],dst_ip[1],dst_ip[2],dst_ip[3]);
				
		m_IP->SetSrcIPAddress(src_ip);
		m_IP->SetDstIPAddress(dst_ip);

		int nIndex = m_ComboEnetName.GetCurSel();
		m_NI->SetAdapterNumber(nIndex);

		m_NI->PacketStartDriver();

		SetDlgState( IPC_ADDR_SET ) ;
		SetDlgState( IPC_READYTOSEND ) ;		
	}

	m_bSendReady = !m_bSendReady ;
}

void CIPCAppDlg::OnCheckBroadcast() 
{
	CButton* pChkButton = (CButton*) GetDlgItem( IDC_CHECK_TOALL ) ;

	if ( pChkButton->GetCheck( ) )	SetDlgState( IPC_BROADCASTMODE ) ;
	else							SetDlgState( IPC_UNICASTMODE ) ;
}

void CIPCAppDlg::SetRegstryMessage()
{
	
}

void CIPCAppDlg::SendData()
{
	CString MsgHeader ; 
	if ( m_unDstEnetAddr == (unsigned int)0xff )
		MsgHeader.Format( "[%s:BROADCAST] ", m_unSrcEnetAddr ) ;
	else
		MsgHeader.Format( "[%s:%s] ", m_unSrcEnetAddr, m_unDstEnetAddr ) ;

	m_ListChat.AddString( MsgHeader + m_stMessage ) ;

	int nlength = m_stMessage.GetLength();
	unsigned char* ppayload = new unsigned char[nlength+1];
	memcpy(ppayload,(unsigned char*)(LPCTSTR)m_stMessage,nlength);
	ppayload[nlength] = '\0';
	
	m_TCP->SetDestinPort(TCP_PORT_CHAT);

	m_ChatApp->Send(ppayload,m_stMessage.GetLength());
}

BOOL CIPCAppDlg::Receive(unsigned char *ppayload)
{
	CString Msg;
	int len_ppayload = strlen((char *)ppayload); 

	unsigned char *GetBuff = (unsigned char *)malloc(len_ppayload);
	memset(GetBuff,0,len_ppayload);
	memcpy(GetBuff,ppayload,len_ppayload);
	GetBuff[len_ppayload] = '\0';


	if( m_unSrcEnetAddr == (unsigned int)0xff )
		Msg.Format("[%s:BROADCAST] %s",m_unDstEnetAddr,(char *)GetBuff);
	else
		Msg.Format("[%s:%s] %s",m_unDstEnetAddr,m_unSrcEnetAddr,(char *)GetBuff);

	KillTimer(1);
	m_ListChat.AddString( (char*) Msg.GetBuffer(0) ) ;
	return TRUE ;
}

BOOL CIPCAppDlg::PreTranslateMessage(MSG* pMsg) 
{
	switch( pMsg->message )
	{
	case WM_KEYDOWN :
		switch( pMsg->wParam )
		{
		case VK_RETURN : 
			if ( ::GetDlgCtrlID( ::GetFocus( ) ) == IDC_EDIT_MSG ) 
				OnSendMessage( ) ;					return FALSE ;
		case VK_ESCAPE : return FALSE ;
		}
		break ;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CIPCAppDlg::SetDlgState(int state)
{
	UpdateData( TRUE ) ;
	int i;
	CString device_description;

	CButton*	pChkButton = (CButton*) GetDlgItem( IDC_CHECK_TOALL ) ;
	CButton*	pSendButton = (CButton*) GetDlgItem( IDC_BUTTON_SEND ) ;
	CButton*	pSetAddrButton = (CButton*) GetDlgItem( IDC_BUTTON_ADDR ) ;
	CButton*	pFileSearchButton = (CButton*) GetDlgItem( IDC_BUTTON_FILE ) ;
	CButton*	pFileSendButton = (CButton*) GetDlgItem( IDC_BUTTON_FILESEND ) ;

	CEdit*		pMsgEdit = (CEdit*) GetDlgItem( IDC_EDIT_MSG ) ;
	CEdit*		pSrcEdit = (CEdit*) GetDlgItem( IDC_EDIT_SRC ) ;
	CEdit*		pDstEdit = (CEdit*) GetDlgItem( IDC_EDIT_DST ) ;
	CEdit*		pSrcIPEdit = (CEdit*) GetDlgItem( IDC_EDIT_SRCIP ) ;
	CEdit*		pDstIPEdit = (CEdit*) GetDlgItem( IDC_EDIT_DSTIP ) ;
	CEdit*		pFilePathEdit = (CEdit*) GetDlgItem( IDC_EDIT_FilePath ) ;

	CComboBox*	pEnetNameCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ENETADDR);

	switch( state )
	{
	case IPC_INITIALIZING :
		pSendButton->EnableWindow( FALSE ) ;
		pMsgEdit->EnableWindow( FALSE ) ;
		m_ListChat.EnableWindow( FALSE ) ;
		pFilePathEdit->EnableWindow( FALSE );
		pFileSearchButton->EnableWindow( FALSE );
		pFileSendButton->EnableWindow( FALSE );
		break ;
	case IPC_READYTOSEND : 
		pSendButton->EnableWindow( TRUE ) ;
		pMsgEdit->EnableWindow( TRUE ) ;
		m_ListChat.EnableWindow( TRUE ) ;
		pFilePathEdit->EnableWindow( TRUE );
		pFileSearchButton->EnableWindow( TRUE );
		pFileSendButton->EnableWindow( TRUE );
		break ;
	case IPC_WAITFORACK :	break ;
	case IPC_ERROR :		break ;
	case IPC_UNICASTMODE :
		m_unDstEnetAddr.Format("%.2x%.2x%.2x%.2x%.2x%.2x",0x00,0x00,0x00,0x00,0x00,0x01) ;
		pDstEdit->EnableWindow( TRUE ) ;
		break ;
	case IPC_BROADCASTMODE :	
		m_unDstEnetAddr.Format("%.2x%.2x%.2x%.2x%.2x%.2x",0xFF,0xFF,0xFF,0xFF,0xFF,0xFF) ;
		pDstEdit->EnableWindow( FALSE ) ;
		break ;
	case IPC_ADDR_SET :
		pSetAddrButton->SetWindowText( "재설정(&R)" ) ; 
		pSrcEdit->EnableWindow( FALSE ) ;
		pDstEdit->EnableWindow( FALSE ) ;
		pChkButton->EnableWindow( FALSE ) ;
		pSrcIPEdit->EnableWindow( FALSE );
		pDstIPEdit->EnableWindow( FALSE );
		pEnetNameCombo->EnableWindow( FALSE );
		m_NI->m_thrdSwitch = TRUE;
		break ;
	case IPC_ADDR_RESET :
		pSetAddrButton->SetWindowText( "설정(&O)" ) ; 
		pSrcEdit->EnableWindow( TRUE ) ;
		if ( !pChkButton->GetCheck( ) )
			pDstEdit->EnableWindow( TRUE ) ;
		pChkButton->EnableWindow( TRUE ) ;
		pSrcIPEdit->EnableWindow( TRUE );
		pDstIPEdit->EnableWindow( TRUE );
		pEnetNameCombo->EnableWindow( TRUE );
		m_NI->m_thrdSwitch = FALSE;
		break ;
	case CFT_COMBO_SET :
		for(i=0;i<NI_COUNT_NIC;i++){
			if(!m_NI->GetAdapterObject(i))
				break;
			device_description = m_NI->GetAdapterObject(i)->description;
			device_description.Trim();
			pEnetNameCombo->AddString(device_description);
			pEnetNameCombo->SetCurSel(0);

			PPACKET_OID_DATA OidData;
			OidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));
			OidData->Oid = 0x01010101;
			OidData->Length = 6;

			LPADAPTER adapter = PacketOpenAdapter(m_NI->GetAdapterObject(i)->name);
			PacketRequest(adapter, FALSE, OidData);

			m_unSrcEnetAddr.Format("%.2X%.2X%.2X%.2X%.2X%.2X",OidData->Data[0],OidData->Data[1],OidData->Data[2],OidData->Data[3],OidData->Data[4],OidData->Data[5]) ;
		}
		break;
	}

	UpdateData( FALSE ) ;
}

void CIPCAppDlg::EndofProcess()
{
	m_LayerMgr.DeAllocLayer( ) ;
}

void CIPCAppDlg::OnTimer(UINT nIDEvent) 
{
	KillTimer( 1 ) ;

	CDialog::OnTimer(nIDEvent);
}

void CIPCAppDlg::OnAddFile() 
{
	UpdateData(TRUE);

	CFileDialog dlg( true, "*.*", NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, "All Files(*.*)|*.*|", NULL );
	if( dlg.DoModal() == IDOK )
	{
		m_filePath = dlg.GetPathName();
	}
	UpdateData(FALSE);
}


void CIPCAppDlg::OnSendFile() 
{
	m_TCP->SetDestinPort(TCP_PORT_FILE);
	
	((CFileAppLayer*)GetUnderLayer())->Send( (unsigned char*)(LPCTSTR)m_filePath );
}

void CIPCAppDlg::OnComboEnetAddr()
{
	UpdateData(TRUE);

	int nIndex = m_ComboEnetName.GetCurSel();
	m_NI->GetAdapterObject(nIndex)->name;

	UpdateData(FALSE);
}

void CIPCAppDlg::OnOffFileButton(BOOL bBool)
{
	CButton* pFileSendButton = (CButton*) GetDlgItem( IDC_BUTTON_FILESEND ) ;
	CButton* pFileSearchButton = (CButton*) GetDlgItem( IDC_BUTTON_FILE ) ;
	pFileSendButton->EnableWindow(bBool);
	pFileSearchButton->EnableWindow(bBool);
}