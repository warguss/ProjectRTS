#include "CProtoPacket.h"

CProtoPacket::CProtoPacket()
{
	_proto = new server2N::PacketBody();

	_fd = -1;
	_act = "";
	_type = 0;
	_sector = 0;
	_protoEvent = NULL;
	_protoConnect = NULL;
	_authAgent = false;
	memset(_buffer, '\0', AUTH_BYTE);
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
		LOG_DEBUG("Event Check");
	}
	else if ( _proto->has_connect() )
	{
		server2N::UserConnection connect = _proto->connect();
		if ( connect.contype() == (int32_t)server2N::UserConnection_ConnectionType_TryConnect )
		{
			user->_nickName = connect.mutable_nickname(0)->c_str();
			LOG_DEBUG("SyncUser Connect nicName(%s) nickName(%s)", connect.mutable_nickname(0)->c_str(), user->_nickName.c_str());
		}
	}
	else if ( _proto->has_notice() )
	{
		LOG_DEBUG("Noti Sync");
	} 
	
	return true;
}

bool CProtoPacket::setBuffer(char* buffer)
{
	if ( !_buffer )
	{
		LOG_ERROR("Set Buffer is Null Error (%d)", _fd);
		return false;
	} 
	int32_t length = strlen(buffer);
	if ( length <= 0 )
	{
		LOG_ERROR("Set Buffer Length is invalid (%d)", _fd);
		return false;
	}

	memcpy(_buffer, buffer, length); 

	return true;
}

bool CProtoPacket::clearPacket()
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

	_fd = -1;
	_act = "";
	_type = 0;
	_sector = 0;
	_proto = NULL;
	_protoEvent = NULL;
	_protoConnect = NULL;
	_authAgent = false;
	memset(_buffer, '\0', AUTH_BYTE);

	return true;
} 

bool CProtoPacket::clearForPool()
{
	_fd = -1;
	_act = "";
	_type = 0;
	_sector = 0;
	_authAgent = false;
	memset(_buffer, '\0', AUTH_BYTE);

	return true;
} 

