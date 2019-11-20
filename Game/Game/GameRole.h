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
	/*初始化函数，开发者可以重写该方法实现对象相关的初始化，该函数会在role对象添加到zinxkernel时调用*/
	virtual bool Init()override;

	/*处理信息函数，重写该方法可以对业务数据进行处理，若还需要后续处理流程，则应返回相关数据信息（堆对象）*/
	virtual UserData *ProcMsg(UserData &_poUserData)override;

	/*去初始化函数，类似初始化函数，该函数会在role对象摘除出zinxkernel时调用*/
	virtual void Fini()override;

	/*创建上线时的id和姓名消息*/
    GameMsg *MakeLogonSyncPid();
    /*创建广播聊天消息*/
    GameMsg *MakeTalkBroadcast(std::string _talkContent);
    /*创建广播出生位置消息*/
    GameMsg *MakeInitPosBroadcast();
    /*创建广播移动后新位置消息*/
    GameMsg *MakeNewPosBroadcast();
    /*创建下线时的id和姓名消息*/
    GameMsg *MakeLogoffSyncPid();
    /*创建周围玩家位置消息*/
    GameMsg *MakeSurPlays();
    /*进入场景确认的消息*/
    GameMsg *MakeChangeWorldResponse(int srcId,int targetId);
    /*技能触发消息*/
    GameMsg *MakeSkillTrigger(pb::SkillTrigger *trigger);
    /*技能碰撞消息*/
    GameMsg *MakeSkillContact(pb::SkillContact *contact);

	 //处理玩家移动的消息
    void ProcNewPosition(float _x , float _y , float _z, float _v);
    void ProcTalkContent(std::string content);
    void ProcChangeWorld(int srcId , int targetWorldId);
    void ProcSkillTrigger(pb::SkillTrigger *trigger);
    void ProcSkillContact(pb::SkillContact *contact);

	//处理AOI视野消失
	void ViewDisappear(std::list<AOI_Player*> &oldList, std::list<AOI_Player*> &newList);
	//处理AOI视野出现
	void ViewAppear(std::list<AOI_Player*> &oldList, std::list<AOI_Player*> &newList);


	GameProtocol *mProtocol;
	GameChannel *mChannel;

	//AOI player的接口实现
	virtual int GetX() { return x; };
	//因为unity客户端 地图上使用 x 和 z 轴  ,AOI的y对应的是客户端的z
	virtual int GetY() { return z; };

	//玩家的ID以及名字
	int32_t mPlayerID;
	std::string mPlayerName;

	//表示当前玩家在世界中的坐标
	float x;
	float y;
	float z;
	//玩家面朝东南西北,角度
	float v;

	//玩家的HP值
	int hp;

	//当前玩家所在的AOI世界的指针
	AOI_World *mCurrentWorld;


};

