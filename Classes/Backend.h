/**
 * 三消游戏算法
 * author: zhangpanyi@live.com
 * https://github.com/zhangpanyi/Eliminate
 */

#pragma once

#include <set>
#include <random>
#include <functional>

#include "AStar.h"
#include "Types.h"

class BackendDelegate
{
public:
	/**
	 * 执行消除
	 * @param index 被消除精灵的索引
	 * @param number 当前精灵的编号
	 * @param total 精灵总量
	 */
	virtual void OnEliminate(const MapIndex &index, unsigned int number, unsigned int total) = 0;

	/**
	 * 刷新地图
	 * @param index 刷新显示精灵的索引
	 * @param type 精灵的类型
	 */
	virtual void OnRefreshMap(const MapIndex &index, int type) = 0;


	/**
	 * 精灵落下
	 * @param source 精灵所在的地图索引
	 * @param target 目标索引
	 * @param number 当前精灵的编号
	 * @param total 精灵总量
	 */
	virtual void OnSpriteFalldown(const MapIndex &source, const MapIndex &target, unsigned int number, unsigned int total) = 0;
};

class Backend
{
public:
	enum
	{
		NOTHING = -1,
		NOSPRITE = 0,
	};

	struct Scope
	{
		int min_row;
		int min_col;
		int max_row;
		int max_col;

		void init()
		{
			min_row = min_col = max_row = max_col = -1;
		}

		void update(const MapIndex &index)
		{
			min_row = min_row == -1 ? index.row : index.row < min_row ? index.row : min_row;
			min_col = min_col == -1 ? index.col : index.col < min_col ? index.col : min_col;
			max_row = max_row == -1 ? index.row : index.row > max_row ? index.row : max_row;
			max_col = max_col == -1 ? index.col : index.col > max_col ? index.col : max_col;
		}
	};

public:
	Backend(BackendDelegate *delegate);
	~Backend() = default;

public:
	/**
	 * 重新生成地图
	 */
	void ReGeneration();

	/**
	 * 设置地图
	 * @param config 地图配置
	 */
	void SetMap(const MapConfig &config);

	/**
	 * 获取地图宽度
	 */
	int GetMapWidth() const;

	/**
	 * 获取地图高度
	 */
	int GetMapHeight() const;

	/**
	 * 索引上是否存在有效精灵
	 * @param 精灵索引
	 */
	bool IsValidSprite(const MapIndex &index);

	/**
	 * 索引是否相邻
	 */
	bool IsAdjacent(const MapIndex &a, const MapIndex &b);

	/**
	 * 类型是否相同
	 */
	bool IsSameType(const MapIndex &a, const MapIndex &b);

	/**
	 * 遍历地图
	 * @param step 回调函数
	 * VisitCallback type=-1表示此索引什么都没有, type=0表示没有精灵
	 */
	void VisitMap();

	/**
	 * 交换精灵
	 * @param a 精灵a索引
	 * @param b 精灵b索引
	 */
	void SwapSprite(const MapIndex &a, const MapIndex &b);

	/**
	 * 获取可以消除的移动过的精灵
	 */
	bool GetMovedSpriteAndCanEliminate(std::set<MapIndex> &out);

public:
	/**
	 * 是否可消除
	 * @param out 可消除索引集合
	 * @return bool
	 */
	virtual bool IsCanEliminate(const MapIndex &previous, const MapIndex &current, std::set<MapIndex> &out);

	/**
	 * 执行消除
	 * @param in_elements 将被消除的精灵集合
	 * @reutrn 被消除的精灵数量
	 */
	virtual unsigned int DoEliminate(std::set<MapIndex> &in_elements);

	/**
	 * 落下精灵
	 * @reutrn 是否有精灵落下
	 */
	virtual bool FalldownSprite();

protected:
	/**
	 * 添加精灵到首行
	 * @param 新增精灵的地图索引
	 * @return 补充数量
	 */
	virtual unsigned int AddSpriteToFristLine(std::set<MapIndex> &out);

	/**
	 * 是否可消除
	 * @param out 可消除索引集合
	 * @return bool
	 */
	virtual bool IsCanEliminate(const MapIndex &index, std::set<MapIndex> &out);

private:
	/**
	 * 取随机数
	 */
	int Random(const int min, const int max);

	/**
	 * 计算最短距离
	 */
	int CalculateShortest(const MapIndex &index);

private:
	bool				initialized_;
	BackendDelegate*	delegate_;
	MapConfig			config_;
	Scope				souch_scope_;
	a_star::AStar		a_star_;
	std::mt19937		generator_;
	std::vector<int>	sprites_;
	std::set<MapIndex>	moved_sprites_;
};