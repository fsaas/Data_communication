// ChatAppLayer.cpp: implementation of the CChatAppLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ipc.h"
#include "ChatAppLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChatAppLayer::CChatAppLayer( char* pName ) 
	: CBaseLayer( pName ), 
	mp_Dlg( NULL )
{
	ResetHeader( ) ;
}

CChatAppLayer::~CChatAppLayer()
{
}

void CChatAppLayer::ResetHeader( ) 
{
	m_sHeader.capp_totlen  = 0x0000 ;
	m_sHeader.capp_type    = 0x00 ;

	memset( m_sHeader.capp_data, 0, APP_DATA_SIZE ) ;
}

BOOL CChatAppLayer::Send(unsigned char *ppayload, int nlength)
{
	m_ppayload = ppayload;
	m_length = nlength;
	
	//////////////////////////// fill the blank ///////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	return TRUE;
}

BOOL CChatAppLayer::Receive( unsigned char* ppayload )
{
	PCHAT_APP_HEADER capp_hdr = (PCHAT_APP_HEADER) ppayload ;
	static unsigned char *GetBuff;
	
	if(capp_hdr->capp_totlen <= APP_DATA_SIZE){
		GetBuff = (unsigned char *)malloc(capp_hdr->capp_totlen);
		memset(GetBuff,0,capp_hdr->capp_totlen); 
		memcpy(GetBuff,capp_hdr->capp_data,capp_hdr->capp_totlen); 
		GetBuff[capp_hdr->capp_totlen] = '\0';

		mp_aUpperLayer[0]->Receive((unsigned char*) GetBuff); 
		return TRUE;
	}
	if(capp_hdr->capp_type == DATA_TYPE_BEGIN)
	{	
		GetBuff = (unsigned char *)malloc(capp_hdr->capp_totlen);
		memset(GetBuff,0,capp_hdr->capp_totlen); 
	}
	else if(capp_hdr->capp_type == DATA_TYPE_CONT) 
	{
		strncat((char *)GetBuff,(char *)capp_hdr->capp_data,strlen((char *)capp_hdr->capp_data));
		GetBuff[strlen((char *)GetBuff)] = '\0';
	}
	else if(capp_hdr->capp_type == DATA_TYPE_END)
	{
		memcpy(GetBuff,GetBuff,capp_hdr->capp_totlen);
		GetBuff[capp_hdr->capp_totlen] = '\0';

		mp_aUpperLayer[0]->Receive((unsigned char*) GetBuff);
		free(GetBuff);
	}
	else
		return FALSE;

	return TRUE ;
}

UINT CChatAppLayer::ChatThread( LPVOID pParam )
{
	BOOL bSuccess = FALSE;
	CChatAppLayer *pChat = (CChatAppLayer *)pParam;
	int data_length = APP_DATA_SIZE;
	int seq_tot_num;
	int data_index;	 
	int temp = 0;


	if( pChat->m_length < APP_DATA_SIZE )
		seq_tot_num = 1;
	else
		seq_tot_num = (pChat->m_length/APP_DATA_SIZE)+1;

	for(int i=0;i<=seq_tot_num+1;i++)
	{
		
		if(seq_tot_num==1){
			data_length = pChat->m_length;
		}
		else{
			if(i==seq_tot_num)
				data_length = pChat->m_length%APP_DATA_SIZE;
			else 
				data_length = APP_DATA_SIZE;
		}

		memset(pChat->m_sHeader.capp_data, 0, data_length);

		if(i==0)
		{
			pChat->m_sHeader.capp_totlen = pChat->m_length;
			pChat->m_sHeader.capp_type = DATA_TYPE_BEGIN;
			memset(pChat->m_sHeader.capp_data,0,data_length);
			data_length = 0;
		}
		else if(i!=0 && i<=seq_tot_num) 
		{
			data_index = data_length;
			pChat->m_sHeader.capp_type = DATA_TYPE_CONT;
			pChat->m_sHeader.capp_seq_num = i-1;

			CString str = pChat->m_ppayload;
			str = str.Mid(temp,temp+data_index);

			memcpy(pChat->m_sHeader.capp_data,str,data_length);
			temp += data_index;
		}
		else
		{
			pChat->m_sHeader.capp_type = DATA_TYPE_END;
			memset(pChat->m_ppayload,0,data_length);
			data_length = 0;
		}
		((CTCPLayer*)(pChat->GetUnderLayer()))->SetDestinPort(TCP_PORT_CHAT);
 		bSuccess = pChat->mp_UnderLayer->Send((unsigned char*) &pChat->m_sHeader,data_length+APP_HEADER_SIZE);
	}

	return bSuccess;
}