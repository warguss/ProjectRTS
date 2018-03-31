#include "CProtoPacket.h"


CProtoPacket::CProtoPacket()
{
	_proto = new server2N::PacketBody();

	_fd = -1;
	_type = 0;
}

CProtoPacket::~CProtoPacket()
{
	if ( _proto ) 
	{
		delete _proto;
	} 

	if ( _protoEvent )
	{
		delete _protoEvent;
	}

	if ( _protoConnect )
	{
		delete _protoConnect;
	}

	_proto = NULL;
	_protoEvent = NULL;
	_protoConnect = NULL;
}

bool CProtoPacket::SyncUser(CUser* user)
{
	if ( _proto->has_event() )
	{

	}


	return true;
}	
