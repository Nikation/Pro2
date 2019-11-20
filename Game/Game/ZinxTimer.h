#pragma once
#include "zinx.h"
#include <vector>
#include <list>
//��ʱ��ͨ����,ǿ��1���ӳ�ʱһ��
class ZinxTimer :
	public Ichannel
{
public:
	ZinxTimer();
	~ZinxTimer();
	virtual bool Init()override;
	virtual bool ReadFd(std::string &_input)override;
	virtual bool WriteFd(std::string &_output)override;
	virtual void Fini()override;
	virtual int GetFd()override;
	virtual std::string GetChannelInfo()override;
protected:
	virtual AZinxHandler *GetInputNextStage(BytesMsg &_oInput)override;
	//�������浱ǰchannel��timer �ļ�������
	int m_fd;
};

class TimerOutProc {
public:
	virtual void Proc() = 0;                //��ʱ��ʱ���ȥ���õĽӿ�
	virtual int GetTimerSec() = 0;          //��ȡ��ǰ����ĳ�ʱ�����һ�봥��һ�ξͷ���1
	virtual ~TimerOutProc() {};
};
struct WheelNode{
    int LastCount = -1;						//��ǰ�����Ȧ��
    TimerOutProc *pProc = NULL;				//��ʱ�����ָ��
};

//�̳��Դ�������,������Ϊһ����ʱ����ĵ�����,����ά��һ��ʱ������
class ZinxTimerDeliver :public AZinxHandler
{
	//����
    static ZinxTimerDeliver m_single;

    //��ǰ��ת�̶�
    int m_cur_index = 0;

    //ʱ����������ÿ�������з�һ��list��listԪ����Ȧ���Ͷ�ʱ���ڵ�
    std::vector<std::list<WheelNode>> m_TimerWheel;

public:
    ZinxTimerDeliver();
    ~ZinxTimerDeliver();
	//�����Ļ�ȡ����
    static ZinxTimerDeliver &GetInstance() {
        return m_single;
    }
    //ע��һ��Timer���������
    bool RegisterProcObject(TimerOutProc &_proc);
    //ע��һ��Timer���������
    void UnRegisterProcObject(TimerOutProc &_proc);

    // ͨ�� AZinxHandler �̳�
    virtual IZinxMsg * InternalHandle(IZinxMsg & _oInput) override;
    virtual AZinxHandler * GetNextHandler(IZinxMsg & _oNextMsg) override;
};


