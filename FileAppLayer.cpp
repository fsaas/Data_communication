// FileAppLayer.cpp: implementation of the CFileAppLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ipc.h"
#include "FileAppLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileAppLayer::CFileAppLayer( char* pName )
	: CBaseLayer( pName )
{
	bSEND = TRUE;
	ResetHeader( );
}

CFileAppLayer::~CFileAppLayer()
{
}

void CFileAppLayer::ResetHeader()
{
	m_sHeader.fapp_totlen  = 0x00000000 ;
	m_sHeader.fapp_type    = 0x0000 ;
	m_sHeader.fapp_msg_type = 0x00 ;
	m_sHeader.ed = 0x00;
	m_sHeader.fapp_seq_num = 0x00000000 ;

	memset( m_sHeader.fapp_data, 0, APP_DATA_SIZE) ;
}

BOOL CFileAppLayer::Send(unsigned char* filePath)
{
	m_FilePath = filePath;
	bFILE = TRUE;
	((CIPCAppDlg *)mp_aUpperLayer[0])->OnOffFileButton(FALSE);
	//////////////////////////// fill the blank ///////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	return TRUE;
}

BOOL CFileAppLayer::Receive(unsigned char* ppayload)
{
	static HANDLE hFile = NULL;
	DWORD dwWrite=0, dwState=0;

	int progress_value;

	BOOL bResult;
	BOOL bSuccess = FALSE;
	PFILE_APP_HEADER fapp_hdr = (PFILE_APP_HEADER) ppayload ;
	static unsigned char** GetBuff;

	static int tot_seq_num;					
	static unsigned long curr_chk_seq_num;	
	static int end_chk_num;					

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	if(fapp_hdr->fapp_msg_type == MSG_TYPE_FRAG)
	{
		if(fapp_hdr->fapp_type == DATA_TYPE_BEGIN) 
		{
			((CIPCAppDlg *)mp_aUpperLayer[0])->OnOffFileButton(FALSE);

			hFile = CreateFile((char *)fapp_hdr->fapp_data,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				AfxMessageBox("파일 생성 오류");
				return FALSE;
			}

			GetBuff = (unsigned char**)malloc(sizeof(unsigned char*)*fapp_hdr->fapp_seq_num);
			memset(GetBuff,0,fapp_hdr->fapp_seq_num);
			
			receive_fileTotlen = 0;
			send_fileTotlen = fapp_hdr->fapp_totlen;		
			tot_seq_num = fapp_hdr->fapp_seq_num;		
			curr_chk_seq_num = 0;
			end_chk_num = 0;

			bFILE = FALSE;
			bACK = TRUE;
			AfxBeginThread(FileThread,this);
		}
		else if(fapp_hdr->fapp_type == DATA_TYPE_CONT) 
		{
			receive_fileTotlen += fapp_hdr->fapp_totlen;

			GetBuff[fapp_hdr->fapp_seq_num] = (unsigned char*)malloc(sizeof(unsigned char*)*fapp_hdr-> fapp_totlen);
			memset(GetBuff[fapp_hdr->fapp_seq_num],0,fapp_hdr->fapp_totlen); // 버퍼 초기화
			memcpy(GetBuff[fapp_hdr->fapp_seq_num],fapp_hdr->fapp_data,fapp_hdr->fapp_totlen);
			GetBuff[fapp_hdr->fapp_seq_num][fapp_hdr->fapp_totlen] = '\0';
			
			if(fapp_hdr->fapp_seq_num < curr_chk_seq_num){
				end_chk_num++;
				bACK = TRUE;
			}

			else if(fapp_hdr->fapp_seq_num > curr_chk_seq_num)
			{
				if(GetBuff[curr_chk_seq_num]==NULL)
				{
					nak_num = curr_chk_seq_num;
					bNAK = TRUE;
				}
			}
			else
				end_chk_num++;

			curr_chk_seq_num++;

			progress_value = 100 * ((float)receive_fileTotlen / send_fileTotlen);
			((CIPCAppDlg *)mp_aUpperLayer[0])->m_ProgressCtrl.SetPos(progress_value);

		}
		else if(fapp_hdr->fapp_type == DATA_TYPE_END) // 데이터 끝 부분
		{
			if(send_fileTotlen == receive_fileTotlen)
			{
				int length;
				for(int i=0;i<tot_seq_num;i++){
					if(i<tot_seq_num-1)
						length = FILE_READ_SIZE;
					else
						length = receive_fileTotlen%FILE_READ_SIZE;

					bResult = WriteFile(hFile,GetBuff[i],length,&dwWrite,NULL);
				}
				bSuccess = TRUE;
			}
			
			CloseHandle(hFile);

			if(bSuccess==TRUE)
				AfxMessageBox("파일 수신 완료");
			else
				AfxMessageBox("파일 수신 실패");

			((CIPCAppDlg *)mp_aUpperLayer[0])->m_ProgressCtrl.SetPos(0);
			((CIPCAppDlg *)mp_aUpperLayer[0])->OnOffFileButton(TRUE);
			
			bFILE = TRUE; 
		}
	}
	else if(fapp_hdr->fapp_msg_type == MSG_TYPE_ACK)
	{
		bSEND = TRUE;
	}
	else if(fapp_hdr->fapp_msg_type == MSG_TYPE_NAK)
	{
		bNAK_SEND = TRUE;
	}
	return bSuccess ;
}

UINT CFileAppLayer::FileThread( LPVOID pParam )
{
	CFileAppLayer *fapp_hdr = (CFileAppLayer *)pParam;
	BOOL bSuccess = FALSE ;

	HANDLE hFile;
	DWORD dwRead=0, dwFileSize, dwState=0;
	char *pszBuf;
	BOOL bResult;

	int progress_value;
	int i=0, tot_seq_num;
	CString fileName;
	
	static unsigned char** GetBuff;

	if(bFILE==TRUE)
	{
		hFile = CreateFile((char *)fapp_hdr->m_FilePath,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_READONLY,NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			AfxMessageBox("올바른 파일 경로가 아닙니다.");
			((CIPCAppDlg *)fapp_hdr->mp_aUpperLayer[0])->OnOffFileButton(TRUE);
			return FALSE;
		}

		fileName = fapp_hdr->m_FilePath;
		fileName = fileName.Right(fileName.GetLength()-fileName.ReverseFind('\\')-1);

		dwFileSize = GetFileSize(hFile,0);

		pszBuf = (char *)malloc(dwFileSize);
		
	//////////////////////////// fill the blank ///////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////

	}
	else
	{
		if(send_fileTotlen < FILE_READ_SIZE)
			tot_seq_num = 1;
		else 
			tot_seq_num = (send_fileTotlen/FILE_READ_SIZE)+1; 
	}

	while(i<=tot_seq_num+1)
	{
		if(bACK == TRUE)	
		{
			fapp_hdr->m_sHeader.fapp_totlen = 0;
			fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_ACK;	
			((CTCPLayer*)(fapp_hdr->GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE);

			bSuccess = fapp_hdr->mp_UnderLayer->Send((unsigned char*) &fapp_hdr->m_sHeader,FILE_HEADER_SIZE);

			bSEND = FALSE;
			bACK = FALSE;
		}
		if(bNAK == TRUE)
		{
			fapp_hdr->m_sHeader.fapp_totlen = 0;
			fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_NAK;
			fapp_hdr->m_sHeader.fapp_seq_num = nak_num;

			((CTCPLayer*)(fapp_hdr->GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE);

			bSuccess = fapp_hdr->mp_UnderLayer->Send((unsigned char*) &fapp_hdr->m_sHeader,FILE_HEADER_SIZE);

			bSEND = FALSE;
			bNAK = FALSE;
		}
		if(bNAK_SEND == TRUE)
		{
			fapp_hdr->m_sHeader.fapp_type = DATA_TYPE_CONT;
			fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_FRAG;

			int data_length = strlen((char *)GetBuff[fapp_hdr->m_sHeader.fapp_seq_num]);
			((CTCPLayer*)(fapp_hdr->GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE);

			memcpy(fapp_hdr->m_sHeader.fapp_data,GetBuff[fapp_hdr->m_sHeader.fapp_seq_num],data_length);
			bSuccess = fapp_hdr->mp_UnderLayer->Send((unsigned char*) &fapp_hdr->m_sHeader,FILE_HEADER_SIZE);
			memset(fapp_hdr->m_sHeader.fapp_data,0,data_length);

			
			bNAK_SEND = FALSE;
		}
		if(bSEND == TRUE)
		{
			((CTCPLayer*)(fapp_hdr->GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE);
			if(i==0)
			{
	//////////////////////////// fill the blank ///////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
			}
			else if(i!=0 && i<=tot_seq_num) 
			{
	//////////////////////////// fill the blank ///////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////				
			}
			else 
			{
	//////////////////////////// fill the blank ///////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
			}

			bSuccess = fapp_hdr->mp_UnderLayer->Send((unsigned char*) &fapp_hdr->m_sHeader,FILE_HEADER_SIZE+dwRead);
			memset(fapp_hdr->m_sHeader.fapp_data,0,dwRead);
			i++;
		}
	}

	if(bSuccess==TRUE)
		AfxMessageBox("파일 전송 완료");
	else
		AfxMessageBox("파일 전송 실패");

	((CIPCAppDlg *)fapp_hdr->mp_aUpperLayer[0])->m_ProgressCtrl.SetPos(0);
	((CIPCAppDlg *)fapp_hdr->mp_aUpperLayer[0])->OnOffFileButton(TRUE);

	for(int i = 0; i<tot_seq_num;i++)
	{
		free(GetBuff[i]);
	}
	free(GetBuff);
	free(pszBuf);
	return bSuccess ;
}
