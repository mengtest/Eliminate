#pragma once

#include <vector>

/* 无效索引 */
static const int INVALID_INDEX = -1;

/* 地图配置 */
struct MapConfig
{
	int					height;	// 地图行数
	int					width;	// 地图列数
	int					type;	// 类型数量
	std::vector<bool>	data;	// 有效区域
};

/* 地图索引 */
struct MapIndex
{
	int					row;
	int					col;

	MapIndex() : row(0), col(0) {}

	MapIndex(int row, int col) : row(row), col(col) {}

	bool operator< (const MapIndex &that) const
	{
		return this->row < that.row ? true : this->row == that.row ? this->col < that.col : false;
	}
};