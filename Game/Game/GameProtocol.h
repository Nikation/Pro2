#pragma once
#include "zinx.h"
class GameChannel;
class GameRole;
class GameProtocol :
	public Iprotocol
{
public:
	GameProtocol();
	virtual ~GameProtocol();

	/*ԭʼ���ݺ�ҵ�������໥��������������д�ú�����ʵ��Э��*/
	virtual UserData *raw2request(std::string _szInput)override;
	
	/*ԭʼ���ݺ�ҵ�������໥��������������д�ú�����ʵ��Э��*/
	virtual std::string *response2raw(UserData &_oUserData)override;

	GameChannel *mChannel;
	GameRole *mRole;


protected:
    /*��ȡ�����ɫ��������������Ӧ����д�ú���������ָ����ǰ�������û�������ϢӦ�ô��ݸ��ĸ���ɫ����*/
	virtual Irole *GetMsgProcessor(UserDataMsg &_oUserDataMsg)override;

	/*��ȡ����ͨ��������������Ӧ����д�ú���������ָ����ǰ�ֽ���Ӧ�����ĸ�ͨ�����󷢳�*/
	virtual Ichannel *GetMsgSender(BytesMsg &_oBytes)override;


	//�洢�ϴ��������������ݵ�buf
	std::string mLastBuff;

};

