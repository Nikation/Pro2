#include "ZinxTimer.h"
#include <sys/timerfd.h>
#include <iostream>
using namespace std;
ZinxTimer::ZinxTimer()
	:m_fd(-1)
{
}
ZinxTimer::~ZinxTimer()
{
	if (m_fd >= 0)
	{
		close(m_fd);
		m_fd = -1;
	}
}

bool ZinxTimer::Init()
{
	//������ʱ��,�趨��ʱΪ1��
	int fd = timerfd_create(CLOCK_MONOTONIC,0);
	if (fd < 0)
	{
		return false;
	}
    struct itimerspec spec = {{1,0},{1,0}};
    int ret = timerfd_settime(fd,0,&spec,NULL);
	if (ret < 0)
	{
		close(fd);
		return false;
	}
	m_fd = fd;
	return true;
}

bool ZinxTimer::ReadFd(std::string & _input)
{
	//EPOLLIN������ʱ��,��ʵ���ǳ�ʱ��ʱ��
	//����ʱ������ȡ�������ݸ���һ��������
	uint64_t over_times = 0;
	int ret = read(m_fd, &over_times, sizeof(over_times));
	if (ret != sizeof(over_times))
	{
		return false;
	}
	_input.append((char*)&over_times, sizeof(over_times));
	return true;
}

bool ZinxTimer::WriteFd(std::string & _output)
{
	//���ﲻ��Ҫʵ���߼�
	return false;
}

void ZinxTimer::Fini()
{
	//���ٵ�ǰͨ���Ķ�ʱ��
	if (m_fd >= 0)
	{
		close(m_fd);
		m_fd = -1;
	}
}

int ZinxTimer::GetFd()
{
	return m_fd;
}

std::string ZinxTimer::GetChannelInfo()
{
	return "ZinxTimerChannel";
}

AZinxHandler * ZinxTimer::GetInputNextStage(BytesMsg & _oInput)
{
	return &ZinxTimerDeliver::GetInstance();
}

#define ZINX_TIMER_WHEEL_SIZE 8
ZinxTimerDeliver ZinxTimerDeliver::m_single;
ZinxTimerDeliver::ZinxTimerDeliver()
	:m_cur_index(0)
{
	//��������,ָ�����ӵĸ�����
	m_TimerWheel.resize(ZINX_TIMER_WHEEL_SIZE);
}

ZinxTimerDeliver::~ZinxTimerDeliver()
{
	//�ͷ����еĶ�ʱ����
	for (auto &l : m_TimerWheel)
	{
		for (auto p : l)
		{
			delete p.pProc;
		}
	}
}

bool ZinxTimerDeliver::RegisterProcObject(TimerOutProc & _proc)
{
	//ע��һ����ʱ������Ӧ�����ӵĸ������
	//��ȡ�����ʱ����
	int timespec = _proc.GetTimerSec();
	//������,ʱ��������Ϊ1
	if (timespec < 1)
	{
		return false;
	}
	//���������
	//ע�ᵽ�ĸ����ӵļ��� = ����ǰ�̶� + ����ʱ����) % ������
	int idx = (m_cur_index + timespec) % m_TimerWheel.size();
	
	//����Ȧ��
	//Ȧ�� = ����ʱ���� / ������
	int round = timespec / m_TimerWheel.size();

	//����ڵ�
	WheelNode node;
	node.LastCount = round;
	node.pProc = &_proc;

	m_TimerWheel[idx].push_back(node);

	return true;
}

void ZinxTimerDeliver::UnRegisterProcObject(TimerOutProc & _proc)
{
	//�������и���,�����������
	for (std::list<WheelNode> &l : m_TimerWheel)		//C++11�����������淨
	{
		for (auto it = l.begin(); it != l.end(); )
		{
			//��ɾ���߱���
			if (it->pProc == &_proc)
			{
				it = l.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}

IZinxMsg * ZinxTimerDeliver::InternalHandle(IZinxMsg & _oInput)
{
	BytesMsg &bytes = dynamic_cast<BytesMsg&>(_oInput);
	uint64_t over_times;
	bytes.szData.copy((char*)&over_times, sizeof(over_times));

	//ÿ���Ӷ��ᱻ����

	//���һ�δ����Ĺ����з���,��ʱ������ֹһ��,��ʱ��Ҫ�����ĸ��ӾͲ�ֹһ��,����ѭ��
	for (uint64_t i = 0; i < over_times; ++i)
	{
		//ÿ�δ�����ʱ��̶ȶ�����
		m_cur_index = (m_cur_index + 1) % m_TimerWheel.size();
		std::list<TimerOutProc*> registerList;

		//������ǰ���ӵ�����
		for (auto it = m_TimerWheel[m_cur_index].begin(); it != m_TimerWheel[m_cur_index].end(); )
		{
			//�����Ȧ��Ҫ�Լ�
			it->LastCount--;
			if (it->LastCount < 0)
			{
				//��������	
				it->pProc->Proc();
				//�������֮��,����ǰ�����޳���
				registerList.push_back(it->pProc);
				it = m_TimerWheel[m_cur_index].erase(it);
			}
			else
			{
				++it;
			}
		}

		//����ע��ղ��޳���������
		for (auto p : registerList)
		{
			this->RegisterProcObject(*p);
		}
	}
	return nullptr;
}

AZinxHandler * ZinxTimerDeliver::GetNextHandler(IZinxMsg & _oNextMsg)
{
	return nullptr;
}
