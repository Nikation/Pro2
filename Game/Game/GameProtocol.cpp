#include "GameProtocol.h"
#include "GameChannel.h"
#include "GameRole.h"
#include <iostream>
#include "GameMsg.h"
using namespace std;

//在protocol这一层来做到自己释放
class GoogleGlobalGarbo
{
public:
	~GoogleGlobalGarbo()
	{
		google::protobuf::ShutdownProtobufLibrary();
	}
}gGoogleGarbo;


GameProtocol::GameProtocol()
{
}


GameProtocol::~GameProtocol()
{
}

UserData * GameProtocol::raw2request(std::string _szInput)
{
	//输出从 GameChannel 发送过来的消息
	//cout << _szInput << endl;

	GameMsg *retMsg = nullptr;
	//1 先将读到的数据拼凑到lastbuf
	mLastBuff.append(_szInput);
	while(mLastBuff.size() >= 8)
	{
		//2 读取前面4字节消息长度 (小端字节序)
		int messageLen = mLastBuff[0] |
			mLastBuff[1] << 8 |
			mLastBuff[2] << 16 |
			mLastBuff[3] << 24;

		//3 读取中间4字节的消息ID
		int messageId = mLastBuff[4] |
			mLastBuff[5] << 8 |
			mLastBuff[6] << 16 |
			mLastBuff[7] << 24;
		if (mLastBuff.size() >= messageLen + 8)
		{
			//粘包,提取消息
			string msgContent = mLastBuff.substr(8, messageLen);
			//提取消息之后要从lastbuf中清掉这条消息
			mLastBuff.erase(0, 8 + messageLen);
			cout << "MessageLen:" << messageLen << " MessageId:" << messageId << endl;
			//拿到的消息主体是protobuf的序列化二进制,要通过protobuf来进行反序列化
			GameSingleTLV *tlv = new GameSingleTLV((GameSingleTLV::GameMsgType)messageId, msgContent);
			if (retMsg == nullptr)
				retMsg = new GameMsg;
			retMsg->m_MsgList.push_back(tlv);
		}
		else
		{
			//缺包,不做处理
			break;
		}
	}

	return retMsg;
}

std::string * GameProtocol::response2raw(UserData & _oUserData)
{
	GameMsg &msg = dynamic_cast<GameMsg&>(_oUserData);
	//遍历msg中的链表,提取每一条Single消息,做协议封装
	// |4字节消息长度|4字节消息ID| 消息主体|  (小端字节序)
	string *retBuf = new string;
	for (auto &single : msg.m_MsgList)
	{
		//对protobuf对象进行序列化
		auto protobufString = single->Serialize();

		//消息长度
		int msgLen = protobufString.size();
		//消息ID
		int msgId = single->m_MsgType;

		retBuf->push_back((char)(msgLen & 0xff));
		retBuf->push_back((char)(msgLen>>8 & 0xff));
		retBuf->push_back((char)(msgLen>>16 & 0xff));
		retBuf->push_back((char)(msgLen>>24 & 0xff));

		retBuf->push_back((char)(msgId & 0xff));
		retBuf->push_back((char)(msgId>>8 & 0xff));
		retBuf->push_back((char)(msgId>>16 & 0xff));
		retBuf->push_back((char)(msgId>>24 & 0xff));

		retBuf->append(protobufString);
	}
	return retBuf;
}

Irole * GameProtocol::GetMsgProcessor(UserDataMsg & _oUserDataMsg)
{
	//如果消息是传入的方向,下一个处理者是role对象
	return mRole;
}

Ichannel * GameProtocol::GetMsgSender(BytesMsg & _oBytes)
{
	//如果消息是传出的方向,下一个处理者是channel对象
	return mChannel;
}
