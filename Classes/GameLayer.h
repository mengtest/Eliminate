/**
 * 操作层
 * author: zhangpanyi@live.com
 * https://github.com/zhangpanyi/Eliminate
 */

#pragma once

#include <map>
#include <array>
#include <vector>
#include "AStar.h"
#include "Types.h"
#include "Backend.h"
#include "cocos2d.h"

class GameLayer final : public cocos2d::Layer, public BackendDelegate
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

private:
	virtual void OnEliminate(const MapIndex &index, unsigned int number, unsigned int total) override;
	virtual void OnRefreshMap(const MapIndex &index, int type) override;
	virtual void OnSpriteFalldown(const MapIndex &source, const MapIndex &target, unsigned int number, unsigned int total) override;

private:
	/**
	 * 初始化地板
	 */
	void InitFloor();

	/**
	 * 初始化元素
	 */
	void InitElements();

	/**
	 * 交换元素位置
	 */
	void SwapElementPosition();

	/**
	 * 更改完成
	 */
	void OnChangeFinished();

	/**
	 * 完成位置交换
	 */
	void SwapElementPositionFinished();
	
private:
	/* a*算法 */
	a_star::AStar							a_star_;
	/* 触摸锁 */
	bool									touch_lock_;
	/* 核心算法 */
	Backend									backend_;
	/* 地板元素 */
	std::vector<cocos2d::Sprite*>			floor_elments;
	/* 使用的元素 */
	std::map<MapIndex, cocos2d::Sprite*>	used_elments;
	/* 闲置的元素 */
	std::vector<cocos2d::Sprite *>			free_elements;
	/* 批量渲染 */
	cocos2d::SpriteBatchNode*				batch_node_;
	/* 上次选取的索引 */
	MapIndex								previous_selected_;
	/* 当前选取的索引 */
	MapIndex								current_selected_;
};