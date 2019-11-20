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


	//初始化格子
	this->m_grids.reserve(Xcnt*Ycnt);
	for (auto i = 0; i < Xcnt*Ycnt; ++i)
	{
		//添加 n*m个格子
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
	//获取周围9个格子的玩家信息
	//先计算玩家行号和列号
	int row = (_player->GetY() - minY) / ((maxY - minY) / Ycnt);
	int col = (_player->GetX() - minX) / ((maxX - minX) / Xcnt);
	//周围九个格子的行和列号的组合
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
	//根据以上行和列号的组合,计算格子下标,获取对应格子的所有玩家,进行返回
	std::list<AOI_Player*> retPlayers;
	for (auto &p : row_col)
	{
		//先做保护	
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
	//先计算玩家在第几个格子,然后进行添加
	int idx = Calculate_grid_idx(_player->GetX(), _player->GetY());
	cout << "Player add to gird:" << idx << endl;
	if (idx >= 0 && idx < m_grids.size())
	{
		m_grids[idx]->m_players.push_back(_player);
	}
}

void AOI_World::DelPlayer(AOI_Player * _player)
{
	//先计算玩家在第几个格子,然后进行删除
	int idx = Calculate_grid_idx(_player->GetX(), _player->GetY());
	if (idx >= 0 && idx < m_grids.size())
	{
		m_grids[idx]->m_players.remove(_player);
	}
}

bool AOI_World::GridChanged(AOI_Player * _player, int _newX, int _newY)
{
	//判断当前玩家有没有改变格子
	//_player中存储的还是旧的坐标,  新的坐标是 _newX _newY

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
