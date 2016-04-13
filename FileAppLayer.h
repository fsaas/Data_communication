// FileAppLayer.h: interface for the CEthernetLayer class.
//
//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseLayer.h"
#include "IPLayer.h"
#include "TCPLayer.h"
#include "IPCAppDlg.h"

static BOOL bSEND;
static BOOL bACK; 
static BOOL bNAK;
static BOOL bNAK_SEND;
static BOOL bFILE;
static int send_fileTotlen; 
static int nak_num;

class CFileAppLayer 
: public CBaseLayer
{
private:
	inline void		ResetHeader( );
	CObject* mp_Dlg ;

public:
	unsigned char	*m_ppayload;
	int				m_length;
	unsigned char	*m_FilePath;
	int				receive_fileTotlen;

	BOOL			Receive( unsigned char* ppayload );
	BOOL			Send( unsigned char* filePath );
	
	static UINT		FileThread( LPVOID pParam );	

	CFileAppLayer( char* pName );
	virtual ~CFileAppLayer();

	typedef struct _FILE_APP_HEADER {
		unsigned long	fapp_totlen ;					
		unsigned short	fapp_type ;						
		unsigned char	fapp_msg_type ;					
		unsigned char	ed;								
		unsigned long	fapp_seq_num ;					
		unsigned char	fapp_data[ APP_DATA_SIZE ] ;	
	} FileAppLayer_HEADER, *PFILE_APP_HEADER ;

protected:
	FileAppLayer_HEADER	m_sHeader ;

	enum {			DATA_TYPE_BEGIN = 0x00,	
					DATA_TYPE_CONT = 0x01,	
					DATA_TYPE_END =  0x02 };

	enum {			MSG_TYPE_FRAG = 0x00,	
					MSG_TYPE_ACK = 0x01,	
					MSG_TYPE_NAK = 0x02 };	
};




