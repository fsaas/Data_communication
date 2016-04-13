// TCPLayer.cpp: implementation of the CTCPLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ipc.h"
#include "TCPLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTCPLayer::CTCPLayer( char* pName )
: CBaseLayer( pName )
{
	ResetHeader( ) ;
}

CTCPLayer::~CTCPLayer()
{
}

void CTCPLayer::ResetHeader()
{
	m_sHeader.tcp_sport = 0x0000;
	m_sHeader.tcp_dport = 0x0000;
	m_sHeader.tcp_seq = 0x00000000;
	m_sHeader.tcp_ack = 0x00000000;
	m_sHeader.tcp_offset = 0x00;
	m_sHeader.tcp_flag = 0x00;
	m_sHeader.tcp_window = 0x0000;
	m_sHeader.tcp_cksum = 0x0000;
	m_sHeader.tcp_urgptr = 0x0000;
	memset(m_sHeader.tcp_data,0,TCP_DATA_SIZE);
}

void CTCPLayer::SetSourcePort(unsigned int src_port)
{
	m_sHeader.tcp_sport = src_port;
}

void CTCPLayer::SetDestinPort(unsigned int dst_port)
{
	m_sHeader.tcp_dport = dst_port;
}

BOOL CTCPLayer::Send(unsigned char* ppayload, int nlength)
{
	memcpy( m_sHeader.tcp_data, ppayload, nlength ) ;
	
	BOOL bSuccess = FALSE ;
	bSuccess = mp_UnderLayer->Send((unsigned char*)&m_sHeader,nlength+TCP_HEADER_SIZE);	
	
 	return bSuccess;
}

BOOL CTCPLayer::Receive(unsigned char* ppayload)
{
	PTCPLayer_HEADER pFrame = (PTCPLayer_HEADER) ppayload ;
	
	BOOL bSuccess = FALSE;
	//////////////////////////// fill the blank ///////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////

	return bSuccess ;
}