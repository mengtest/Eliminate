/**************************************************
游戏配置
**************************************************/

#pragma once

#include "Types.h"
#include "Misc/Singleton.h"

class Config final : public Singleton < Config >
{
	SINGLETON(Config);

public:
	/* 获取类型数量 */
	int GetTypeQuantity() const
	{
		return type_quantity_;
	}

	/* 获取元素宽度 */
	int GetElementWidth() const
	{
		return element_width_;
	}

	/* 获取元素高度 */
	int GetElementHeight() const
	{
		return element_height_;
	}

	/* 获取元素落下时间 */
	float GetElementFalldownTime() const
	{
		return fall_down_time_;
	}

	/* 获取元素移动时间 */
	float GetElementMoveTime() const
	{
		return move_time_;
	}

	/* 读取地图配置文件 */
	MapConfig ReadMapConfig(const std::string &filename);

private:
	/* 读取配置文件 */
	void ReadConfigFile();

private:
	float move_time_;
	float fall_down_time_;
	int element_width_;
	int element_height_;
	int type_quantity_;
};