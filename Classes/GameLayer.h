/**************************************************
游戏操作层/核心算法
**************************************************/
#pragma once

#include <map>
#include <array>
#include <vector>
#include "AStar.h"
#include "Types.h"
#include "cocos2d.h"

class Element;

class GameLayer final : public cocos2d::Layer
{
public:
	GameLayer();
	~GameLayer();

	virtual bool init() override;

	CREATE_FUNC(GameLayer);

public:
	/* 触摸事件 */
	virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event);
	virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event);

public:
	/**
	 * 设置地图
	 * @param map_config 地图配置
	 */
	void SetMap(const MapConfig &map_config);

private:
	/**
	 * 获取首行
	 */
	int GetFirstLine();

	/**
	 * 获取起点坐标
	 * @return 起点坐标
	 */
	cocos2d::Vec2 GetStartPoint() const;

	/**
	 * 转换到全局坐标
	 */
	cocos2d::Vec2 ConvertToPosition(const MapIndex &index) const;

	/**
	 * 转换到地图索引
	 * @param position 世界坐标
	 * @return 对应的地图索引
	 */
	MapIndex ConvertToMapIndex(const cocos2d::Vec2 &position) const;

	/**
	 * 是否相邻
	 */
	bool IsAdjacent(const MapIndex &a, const MapIndex &b);

private:
	/**
	 * 随机元素类型
	 */
	int RandElementType() const;

	/**
	 * 获取可消除集合
	 */
	std::set<MapIndex> GetCanEliminate(const MapIndex &index);

	/**
	 * 获取可消除集合
	 */
	std::set<MapIndex> GetToBeEliminated(Element *a, Element *b);

	/**
	 * 计算最短距离
	 */
	int ShortestDistance(const MapIndex &index);

	/**
	 * 初始化搜索范围
	 */
	void InitSouchScope();

	/**
	 * 计算搜索范围
	 */
	void CalculSouchScope(const MapIndex &index);

	/**
	 * 填充元素
	 */
	void FillElements();
	void FillFristLineElements();

	/**
	 * 移动元素
	 * 将source索引上的元素移动到target索引所在的位置上
	 */
	void MoveElements(const MapIndex &source, const MapIndex &target);

	/**
	 * 执行消除
	 */
	void DoEliminate(const std::set<MapIndex> &elements);

	/**
	 * 交换元素位置
	 */
	void SwapElementPosition(Element *a, Element *b);

	/**
	 * 交换元素位置完成
	 */
	void SwapElementPositionFinished();

private:
	/* a star算法 */
	a_star::AStar					a_star_;
	/* 触摸锁 */
	bool							touch_lock_;
	/* 地图配置 */
	MapConfig						map_config_;
	/* 所有元素 */
	std::map<MapIndex, Element*>	all_elments;
	/* 地板元素 */
	std::vector<cocos2d::Sprite*>	floor_elments;
	/* 批量渲染 */
	cocos2d::SpriteBatchNode*		batch_node_;
	/* 上次选取的元素 */
	Element*						previous_selected_;
	/* 当前选取的元素 */
	Element*						current_selected_;
	/* 闲置的元素 */
	std::vector<Element *>			free_elements;
	/* 移动过的元素 */
	std::set<MapIndex>				moved_elements_;
	/* 搜索范围 */
	std::array<int, 4>				search_scope_;
};