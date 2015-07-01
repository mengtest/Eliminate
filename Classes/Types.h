/**
 * 类型定义
 * author: zhangpanyi@live.com
 * https://github.com/zhangpanyi/Eliminate
 */

#pragma once

#include <vector>

/* 无效索引 */
static const int INVALID_INDEX = -1;

/* 地图配置 */
struct MapConfig
{
	int					height;			// 地图行数
	int					width;			// 地图列数
	int					type_quantity;	// 类型数量
	std::vector<bool>	data;			// 有效区域
};

/* 地图索引 */
struct MapIndex
{
	int					row;
	int					col;

	MapIndex() : row(INVALID_INDEX), col(INVALID_INDEX) {}

	MapIndex(int row, int col) : row(row), col(col) {}

	operator bool() const
	{
		return row != INVALID_INDEX && col != INVALID_INDEX;
	}

	bool operator== (const MapIndex &that) const
	{
		return row == that.row && col == that.col;
	}

	bool operator!= (const MapIndex &that) const
	{
		return row != that.row || col != that.col;
	}

	bool operator< (const MapIndex &that) const
	{
		return this->row < that.row ? true : this->row == that.row ? this->col < that.col : false;
	}
};