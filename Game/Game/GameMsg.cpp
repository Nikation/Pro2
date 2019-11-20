#include "GameMsg.h"
using namespace pb;
using namespace std;

GameSingleTLV::GameSingleTLV(GameMsgType _Type, google::protobuf::Message * _poGameMsg)
{
	//带参数的构造,直接赋值到成员变量中即可
	this->m_MsgType = _Type;
	this->m_poGameMsg = _poGameMsg;
}

GameSingleTLV::GameSingleTLV(GameMsgType _Type, std::string _szInputData)
{
	//需要反序列化的构造函数 _type是消息ID,_szInputData是protobuf的序列化二进制数据
	switch (_Type)
	{
	case GAME_MSG_LOGON_SYNCPID:
	{
		m_poGameMsg = new SyncPid;
		break;
	}
	case GAME_MSG_TALK_CONTENT:
	{
		m_poGameMsg = new Talk;
		break;
	}
	case GAME_MSG_NEW_POSTION:
	{
		m_poGameMsg = new Position;
		break;
	}
	case GAME_MSG_SKILL_TRIGGER:
	{
		m_poGameMsg = new SkillTrigger;
		break;
	}
	case GAME_MSG_SKILL_CONTACT:
	{
		m_poGameMsg = new SkillContact;
		break;
	}
	case GAME_MSG_CHANGE_WORLD:
	{
		m_poGameMsg = new ChangeWorldRequest;
		break;
	}
	case GAME_MSG_BROADCAST:
	{
		m_poGameMsg = new BroadCast;
		break;
	}
	case GAME_MSG_LOGOFF_SYNCPID:
	{
		m_poGameMsg = new SyncPid;
		break;
	}
	case GAME_MSG_SUR_PLAYER:
	{
		m_poGameMsg = new SyncPlayers;
		break;
	}
	case GAME_MSG_SKILL_BROAD:
	{
		m_poGameMsg = new SkillTrigger;
		break;
	}
	case GAME_MSG_SKILL_CONTACT_BROAD:
	{
		m_poGameMsg = new SkillContact;
		break;
	}
	case GAME_MSG_CHANGE_WORLD_RESPONSE:
	{
		m_poGameMsg = new ChangeWorldResponse;
		break;
	}
	default:
		break;
	}
	//反序列化
	if (m_poGameMsg != nullptr)
	{
		m_poGameMsg->ParseFromString(_szInputData);
		m_MsgType = _Type;
	}
}

GameSingleTLV::~GameSingleTLV()
{
	if (m_poGameMsg != nullptr)
		delete m_poGameMsg;
}

std::string GameSingleTLV::Serialize()
{
	if (m_poGameMsg)
	{
		string poRet;
		m_poGameMsg->SerializeToString(&poRet);
		return poRet;
	}
	return std::string();
}


//构造这边不需要做什么事情
GameMsg::GameMsg()
{
}

//析构这边要析构所有的msg
GameMsg::~GameMsg()
{
	for(auto msg : m_MsgList)
	{
		delete msg;
	}
}

