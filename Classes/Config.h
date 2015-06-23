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
	/* 获取元素宽度 */
	int GetElementWidth() const
	{
		return element_width_;
	}

	/* 获取元素高度 */
	int GetElementHeight() const
	{
		return element_height;
	}

	/* 获取元素填充时间 */
	float GetElementFillTime() const
	{
		return fill_time_;
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
	float fill_time_;
	float move_time_;
	int element_width_;
	int element_height;
};