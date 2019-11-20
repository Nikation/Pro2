#include "GameProtocol.h"
#include "GameChannel.h"
#include "GameRole.h"
#include <iostream>
#include "GameMsg.h"
using namespace std;

//��protocol��һ���������Լ��ͷ�
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
	//����� GameChannel ���͹�������Ϣ
	//cout << _szInput << endl;

	GameMsg *retMsg = nullptr;
	//1 �Ƚ�����������ƴ�յ�lastbuf
	mLastBuff.append(_szInput);
	while(mLastBuff.size() >= 8)
	{
		//2 ��ȡǰ��4�ֽ���Ϣ���� (С���ֽ���)
		int messageLen = mLastBuff[0] |
			mLastBuff[1] << 8 |
			mLastBuff[2] << 16 |
			mLastBuff[3] << 24;

		//3 ��ȡ�м�4�ֽڵ���ϢID
		int messageId = mLastBuff[4] |
			mLastBuff[5] << 8 |
			mLastBuff[6] << 16 |
			mLastBuff[7] << 24;
		if (mLastBuff.size() >= messageLen + 8)
		{
			//ճ��,��ȡ��Ϣ
			string msgContent = mLastBuff.substr(8, messageLen);
			//��ȡ��Ϣ֮��Ҫ��lastbuf�����������Ϣ
			mLastBuff.erase(0, 8 + messageLen);
			cout << "MessageLen:" << messageLen << " MessageId:" << messageId << endl;
			//�õ�����Ϣ������protobuf�����л�������,Ҫͨ��protobuf�����з����л�
			GameSingleTLV *tlv = new GameSingleTLV((GameSingleTLV::GameMsgType)messageId, msgContent);
			if (retMsg == nullptr)
				retMsg = new GameMsg;
			retMsg->m_MsgList.push_back(tlv);
		}
		else
		{
			//ȱ��,��������
			break;
		}
	}

	return retMsg;
}

std::string * GameProtocol::response2raw(UserData & _oUserData)
{
	GameMsg &msg = dynamic_cast<GameMsg&>(_oUserData);
	//����msg�е�����,��ȡÿһ��Single��Ϣ,��Э���װ
	// |4�ֽ���Ϣ����|4�ֽ���ϢID| ��Ϣ����|  (С���ֽ���)
	string *retBuf = new string;
	for (auto &single : msg.m_MsgList)
	{
		//��protobuf����������л�
		auto protobufString = single->Serialize();

		//��Ϣ����
		int msgLen = protobufString.size();
		//��ϢID
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
	//�����Ϣ�Ǵ���ķ���,��һ����������role����
	return mRole;
}

Ichannel * GameProtocol::GetMsgSender(BytesMsg & _oBytes)
{
	//�����Ϣ�Ǵ����ķ���,��һ����������channel����
	return mChannel;
}
