#pragma once
#include "zinx.h"
#include "GameMsg.h"
#include "AOI_World.h"
class GameChannel;
class GameProtocol;
class GameRole :
	public Irole,
	public AOI_Player
{
	static int smRoleCount;
public:

	GameRole();
	virtual ~GameRole();
	/*��ʼ�������������߿�����д�÷���ʵ�ֶ�����صĳ�ʼ�����ú�������role������ӵ�zinxkernelʱ����*/
	virtual bool Init()override;

	/*������Ϣ��������д�÷������Զ�ҵ�����ݽ��д���������Ҫ�����������̣���Ӧ�������������Ϣ���Ѷ���*/
	virtual UserData *ProcMsg(UserData &_poUserData)override;

	/*ȥ��ʼ�����������Ƴ�ʼ���������ú�������role����ժ����zinxkernelʱ����*/
	virtual void Fini()override;

	/*��������ʱ��id��������Ϣ*/
    GameMsg *MakeLogonSyncPid();
    /*�����㲥������Ϣ*/
    GameMsg *MakeTalkBroadcast(std::string _talkContent);
    /*�����㲥����λ����Ϣ*/
    GameMsg *MakeInitPosBroadcast();
    /*�����㲥�ƶ�����λ����Ϣ*/
    GameMsg *MakeNewPosBroadcast();
    /*��������ʱ��id��������Ϣ*/
    GameMsg *MakeLogoffSyncPid();
    /*������Χ���λ����Ϣ*/
    GameMsg *MakeSurPlays();
    /*���볡��ȷ�ϵ���Ϣ*/
    GameMsg *MakeChangeWorldResponse(int srcId,int targetId);
    /*���ܴ�����Ϣ*/
    GameMsg *MakeSkillTrigger(pb::SkillTrigger *trigger);
    /*������ײ��Ϣ*/
    GameMsg *MakeSkillContact(pb::SkillContact *contact);

	 //��������ƶ�����Ϣ
    void ProcNewPosition(float _x , float _y , float _z, float _v);
    void ProcTalkContent(std::string content);
    void ProcChangeWorld(int srcId , int targetWorldId);
    void ProcSkillTrigger(pb::SkillTrigger *trigger);
    void ProcSkillContact(pb::SkillContact *contact);

	//����AOI��Ұ��ʧ
	void ViewDisappear(std::list<AOI_Player*> &oldList, std::list<AOI_Player*> &newList);
	//����AOI��Ұ����
	void ViewAppear(std::list<AOI_Player*> &oldList, std::list<AOI_Player*> &newList);


	GameProtocol *mProtocol;
	GameChannel *mChannel;

	//AOI player�Ľӿ�ʵ��
	virtual int GetX() { return x; };
	//��Ϊunity�ͻ��� ��ͼ��ʹ�� x �� z ��  ,AOI��y��Ӧ���ǿͻ��˵�z
	virtual int GetY() { return z; };

	//��ҵ�ID�Լ�����
	int32_t mPlayerID;
	std::string mPlayerName;

	//��ʾ��ǰ����������е�����
	float x;
	float y;
	float z;
	//����泯��������,�Ƕ�
	float v;

	//��ҵ�HPֵ
	int hp;

	//��ǰ������ڵ�AOI�����ָ��
	AOI_World *mCurrentWorld;


};

