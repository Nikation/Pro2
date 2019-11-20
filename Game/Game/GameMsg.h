#pragma once
#include "zinx.h"
#include "msg.pb.h"
#include <list>

class GameSingleTLV{
public:
	//������Ϸ��Э������,�������ϢID ö��
    enum GameMsgType {
        GAME_MSG_LOGON_SYNCPID = 1,             //ͬ�����id������
        GAME_MSG_TALK_CONTENT = 2,              //������Ϣ
        GAME_MSG_NEW_POSTION = 3,               //ͬ�����λ��
        GAME_MSG_SKILL_TRIGGER = 4,             //���ܴ���
        GAME_MSG_SKILL_CONTACT = 5,             //��������
        GAME_MSG_CHANGE_WORLD = 6,              //�����л�

        GAME_MSG_BROADCAST = 200,               //��ͨ�㲥��Ϣ
        GAME_MSG_LOGOFF_SYNCPID = 201,          //���������Ϣ
        GAME_MSG_SUR_PLAYER = 202,              //ͬ���ܱ������Ϣ
        GAME_MSG_SKILL_BROAD = 204,             //���ܴ����㲥
        GAME_MSG_SKILL_CONTACT_BROAD = 205,     //�������й㲥
        GAME_MSG_CHANGE_WORLD_RESPONSE = 206,   //�л������㲥
    } m_MsgType;

	//�洢����protobuf����Ϣ����
    google::protobuf::Message *m_poGameMsg = NULL;
    GameSingleTLV(GameMsgType _Type, google::protobuf::Message * _poGameMsg);
    GameSingleTLV(GameMsgType _Type, std::string _szInputData);
    ~GameSingleTLV();

    std::string Serialize();
};

//�洢�ӿͻ��˷��͹�������Ϣ,���ߴӷ�����Ҫ���͵��ͻ��˵���Ϣ
class GameMsg :
	public UserData
{
public:
	std::list<GameSingleTLV*> m_MsgList;
	GameMsg();
	virtual ~GameMsg();
};

