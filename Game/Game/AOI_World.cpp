#include "AOI_World.h"
#include <iostream>
using namespace std;

AOI_World:: AOI_World(int _minx, int _maxx, int _miny, int _maxy, int _xcnt, int _ycnt)
{
	this->minX = _minx;
	this->minY = _miny;
	this->maxX = _maxx;
	this->maxY = _maxy;
	this->Xcnt = _xcnt;
	this->Ycnt = _ycnt;


	//��ʼ������
	this->m_grids.reserve(Xcnt*Ycnt);
	for (auto i = 0; i < Xcnt*Ycnt; ++i)
	{
		//��� n*m������
		m_grids.push_back(new AOI_Grid(i));
	}
}


AOI_World::~AOI_World()
{
	for (auto g : this->m_grids)
	{
		delete g;
	}
}

std::list<AOI_Player*> AOI_World::GetSurPlayers(AOI_Player * _player)
{
	//��ȡ��Χ9�����ӵ������Ϣ
	//�ȼ�������кź��к�
	int row = (_player->GetY() - minY) / ((maxY - minY) / Ycnt);
	int col = (_player->GetX() - minX) / ((maxX - minX) / Xcnt);
	//��Χ�Ÿ����ӵ��к��кŵ����
	pair<int, int> row_col[] =
	{
		make_pair(row - 1,col - 1),
		make_pair(row - 1, col),
		make_pair(row - 1,col + 1),
		make_pair(row ,col - 1),
		make_pair(row , col),
		make_pair(row ,col + 1),
		make_pair(row + 1,col - 1),
		make_pair(row + 1, col),
		make_pair(row + 1,col + 1),
	};
	//���������к��кŵ����,��������±�,��ȡ��Ӧ���ӵ��������,���з���
	std::list<AOI_Player*> retPlayers;
	for (auto &p : row_col)
	{
		//��������	
		if (p.first < 0 || p.first >= Ycnt)
			continue;
		if (p.second < 0 || p.second >= Xcnt)
			continue;
		int idx = p.first * Xcnt + p.second;
		//cout << "row:" << p.first << " col:" << p.second << " idx:" << idx << endl;
		for (auto &i : m_grids[idx]->m_players)
		{
			retPlayers.push_back(i);
		}
	}
	return retPlayers;
}

void AOI_World::AddPlayer(AOI_Player * _player)
{
	//�ȼ�������ڵڼ�������,Ȼ��������
	int idx = Calculate_grid_idx(_player->GetX(), _player->GetY());
	cout << "Player add to gird:" << idx << endl;
	if (idx >= 0 && idx < m_grids.size())
	{
		m_grids[idx]->m_players.push_back(_player);
	}
}

void AOI_World::DelPlayer(AOI_Player * _player)
{
	//�ȼ�������ڵڼ�������,Ȼ�����ɾ��
	int idx = Calculate_grid_idx(_player->GetX(), _player->GetY());
	if (idx >= 0 && idx < m_grids.size())
	{
		m_grids[idx]->m_players.remove(_player);
	}
}

bool AOI_World::GridChanged(AOI_Player * _player, int _newX, int _newY)
{
	//�жϵ�ǰ�����û�иı����
	//_player�д洢�Ļ��Ǿɵ�����,  �µ������� _newX _newY

	int oldIdx = Calculate_grid_idx(_player->GetX(), _player->GetY());
	int newIdx = Calculate_grid_idx(_newX, _newY);
	return oldIdx!=newIdx;
}

int AOI_World::Calculate_grid_idx(int x, int y)
{
	int row = (y - minY) / ((maxY - minY) / Ycnt);
	int col = (x - minX) / ((maxX - minX) / Xcnt);
	int index = row * Xcnt + col;
	return index;
}
