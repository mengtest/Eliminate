#include "Backend.h"

#include <random>
#include <cassert>
#include <algorithm>

Backend::Backend(BackendDelegate *delegate)
	: initialized_(false)
	, delegate_(delegate)
{
	assert(delegate_);
}

// 设置地图
void Backend::SetMap(const MapConfig &config)
{
	if (config.data.size() == config.width * config.height)
	{
		config_ = config;
		sprites_.clear();
		moved_sprites_.clear();
		souch_scope_.init();
		initialized_ = true;
		ReGeneration();
		VisitMap();
	}
	else
	{
		throw std::runtime_error("Invalid map configuration!");
	}
}

// 获取地图宽度
int Backend::GetMapWidth() const
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}
	return config_.width;
}

// 获取地图高度
int Backend::GetMapHeight() const
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}
	return config_.height;
}

// 随机精灵类型
int Backend::RandSpriteType() const
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, config_.type_quantity);
	return dis(gen);
}

// 精灵是否相邻
bool Backend::IsAdjacent(const MapIndex &a, const MapIndex &b)
{
	for (int row = a.row - 1; row <= a.row + 1; ++row)
	{
		for (int col = a.col - 1; col <= a.col + 1; ++col)
		{
			if (abs((a.row - b.row)) + abs((a.col - b.col)) == 1)
			{
				return true;
			}
		}
	}
	return false;
}

// 类型是否相同
bool Backend::IsSameType(const MapIndex &a, const MapIndex &b)
{
	if (!IsValidSprite(a) || !IsValidSprite(b))
	{
		throw std::runtime_error("invalid element index!");
	}

	return sprites_[a.row * config_.width + a.col] == sprites_[b.row * config_.width + b.col];
}

// 计算最短距离
int Backend::CalculateShortest(const MapIndex &index)
{
	// 求最短距离
	std::vector<int> heap;
	for (int col = 0; col < config_.width; ++col)
	{
		if (config_.data[col])
		{
			a_star::AStarParam param;
			param.start_point.row = 0;
			param.start_point.col = col;
			param.end_point.row = index.row;
			param.end_point.col = index.col;
			param.total_row = config_.height;
			param.total_col = config_.width;
			param.is_can_reach = [&](const a_star::Vec2 &point)
			{
				return config_.data[point.row * config_.width + point.col];
			};

			int distance = a_star_.Search(param).size();
			if (distance > 0)
			{
				heap.push_back(distance);
				std::push_heap(heap.begin(), heap.end(), [](int a, int b)->bool
				{
					return a > b;
				});
			}
		}
	}
	return heap.size() > 0 ? heap[0] : ~0;
}

// 重新生成地图
void Backend::ReGeneration()
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	for (int idx = 0; idx < config_.width * config_.height; ++idx)
	{
		sprites_.push_back(config_.data[idx] ? RandSpriteType() : NOTHING);
	}
}

// 有效精灵索引
bool Backend::IsValidSprite(const MapIndex &index)
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	if (index.col >= 0 && index.row >= 0 && index.col < config_.width && index.row < config_.height)
	{
		int type = sprites_[index.row * config_.width + index.col];
		return (type != NOTHING) && (type != NOSPRITE);
	}
	return false;
}

// 遍历地图
void Backend::VisitMap()
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	const size_t max_size = config_.width * config_.height;
	for (size_t idx = 0; idx < max_size; ++idx)
	{
		delegate_->OnRefreshMap(MapIndex(idx / config_.width, idx % config_.width), sprites_[idx]);
	}
}

// 交换精灵
void Backend::SwapSprite(const MapIndex &a, const MapIndex &b)
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	souch_scope_.init();

	if (!IsValidSprite(a) || !IsValidSprite(b))
	{
		throw std::runtime_error("invalid element index!");
	}
	std::swap(sprites_[a.row * config_.width + a.col], sprites_[b.row * config_.width + b.col]);
}

// 移动过的是否可消除精灵
bool Backend::GetMovedSpriteAndCanEliminate(std::set<MapIndex> &out)
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	out.clear();
	std::set<MapIndex> eliminate_set;
	for (auto &index : moved_sprites_)
	{
		if (sprites_[index.row * config_.width + index.col] > NOSPRITE && IsCanEliminate(index, eliminate_set))
		{
			for (auto can_eliminate_index : eliminate_set)
			{
				out.insert(can_eliminate_index);
			}		
		}
	}
	moved_sprites_.clear();
	return out.empty() == false;
}

// 是否可消除
bool Backend::IsCanEliminate(const MapIndex &index, std::set<MapIndex> &out)
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	out.clear();
	if (IsValidSprite(index))
	{
		std::set<MapIndex> elements_set;
		const int type = sprites_[index.row * config_.width + index.col];

		// 横向遍历
		for (int col = 0; col < config_.width; ++col)
		{
			const int idx = index.row * config_.width + col;
			if (sprites_[idx] == type)
			{
				elements_set.insert(MapIndex(index.row, col));
			}
			else
			{
				if (elements_set.size() >= 3 && (elements_set.find(index) != elements_set.end()))
				{
					break;
				}
				elements_set.clear();
			}
		}
		if (elements_set.size() < 3) elements_set.clear();

		// 纵向遍历
		for (int row = 0; row < config_.height; ++row)
		{
			const int idx = row * config_.width + index.col;
			if (sprites_[idx] == type)
			{
				out.insert(MapIndex(row, index.col));
			}
			else
			{
				if (out.size() >= 3 && (out.find(index) != out.end()))
				{
					break;
				}
				out.clear();
			}
		}
		if (out.size() < 3) out.clear();

		for (auto &idx : elements_set)
		{
			out.insert(idx);
		}

		return out.empty() == false;
	}
	return false;
}

bool Backend::IsCanEliminate(const MapIndex &previous, const MapIndex &current, std::set<MapIndex> &out)
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	out.clear();
	if (IsSameType(current, previous))
	{
		return false;
	}

	std::set<MapIndex> elements_set;
	IsCanEliminate(current, out);
	IsCanEliminate(previous, elements_set);
	for (auto &index : elements_set)
	{
		out.insert(index);
	}
	return out.empty() == false;
}

// 执行消除
unsigned int Backend::DoEliminate(std::set<MapIndex> &in_elements)
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	unsigned int count = 0;
	for (auto &index : in_elements)
	{
#ifdef _DEBUG
		if (!IsValidSprite(index))
		{
			throw std::runtime_error("invalid element index!");
		}
#endif
		sprites_[index.row * config_.width + index.col] = NOSPRITE;
		souch_scope_.update(index);
		delegate_->OnEliminate(index, ++count, in_elements.size());
	}
	return count;
}

// 首行添加精灵
unsigned int Backend::AddSpriteToFristLine(std::set<MapIndex> &out)
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	out.clear();
	unsigned int count = 0;
	const size_t max_size = config_.width * config_.height;
	for (size_t idx = 0; idx < max_size; ++idx)
	{
		if (config_.data[idx])
		{
			const int row = idx / config_.width;
			const int base = row * config_.width;
			for (int col = 0; col < config_.width; ++col)
			{
				if (sprites_[base + col] == NOSPRITE)
				{
					sprites_[base + col] = RandSpriteType();
					out.insert(MapIndex(row, col));
					delegate_->OnRefreshMap(MapIndex(row, col), sprites_[base + col]);
					++count;
				}
			}
			return count;
		}
	}
	return count;
}

// 移动路线
struct MoveRoute
{
	MapIndex source;
	MapIndex target;

	MoveRoute(const MapIndex &a, const MapIndex &b)
		: source(a)
		, target(b)
	{
	}
};

// 落下精灵
bool Backend::FalldownSprite()
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	// 补充第一行精灵
	std::set<MapIndex> added_set;
	AddSpriteToFristLine(added_set);

	// 获取首行
	int frist_line = 0;
	for (int idx = 0; idx < config_.width * config_.height; ++idx)
	{
		if (config_.data[idx])
		{
			frist_line = idx / config_.width;
			break;
		}
	}

	// 精灵下落
	unsigned int before_size = 0;
	std::set<MapIndex> moved_set;
	std::vector<MoveRoute> sp_move_route;

	do
	{
		before_size = moved_set.size();

		// 扩大搜索范围
		if (souch_scope_.min_row > 0) --souch_scope_.min_row;
		if (souch_scope_.min_col > 0) --souch_scope_.min_col;
		if (souch_scope_.max_row < config_.height - 1) ++souch_scope_.max_row;
		if (souch_scope_.max_col < config_.width - 1) ++souch_scope_.max_col;

		for (int row = souch_scope_.min_row; row <= souch_scope_.max_row; ++row)
		{
			for (int col = souch_scope_.min_col; col <= souch_scope_.max_col; ++col)
			{
				const int current_idx = row * config_.width + col;
				const int previous_row_idx = (row - 1) * config_.width + col;
				const int next_row_idx = (row + 1) * config_.width + col;

				// 如果此处有精灵并且在此轮中没有被移动过
				if ((sprites_[current_idx] > NOSPRITE) && (moved_set.find(MapIndex(row, col)) == moved_set.end()))
				{
					// 向下补充
					if ((row + 1 < config_.height)
						&& (config_.data[next_row_idx])
						&& (sprites_[next_row_idx] == NOSPRITE))
					{
						moved_set.insert(MapIndex(row + 1, col));
						std::swap(sprites_[current_idx], sprites_[next_row_idx]);
						sp_move_route.push_back(MoveRoute(MapIndex(row, col), MapIndex(row + 1, col)));
					}
					// 横向移动
					else if (row > frist_line)
					{
						// 如果左边是空格并且空格上方没有精灵
						if (col - 1 >= 0
							&& (config_.data[current_idx - 1])
							&& (!config_.data[previous_row_idx - 1])
							&& (sprites_[current_idx - 1] == NOSPRITE))
						{
							// 如果空格的左边是有效格
							if (col - 2 >= 0 && config_.data[current_idx - 2])
							{
								if (CalculateShortest(MapIndex(row, col)) <= CalculateShortest(MapIndex(row, col - 2)))
								{
									moved_set.insert(MapIndex(row, col - 1));
									std::swap(sprites_[current_idx], sprites_[current_idx - 1]);
									sp_move_route.push_back(MoveRoute(MapIndex(row, col), MapIndex(row, col - 1)));
									continue;
								}
								else if (sprites_[current_idx - 2] > NOSPRITE
										 && moved_set.find(MapIndex(row, col - 2)) == moved_set.end())
								{
									moved_set.insert(MapIndex(row, col - 1));
									std::swap(sprites_[current_idx - 2], sprites_[current_idx - 1]);
									sp_move_route.push_back(MoveRoute(MapIndex(row, col - 2), MapIndex(row, col - 1)));
									continue;
								}
							}
							else
							{
								moved_set.insert(MapIndex(row, col - 1));
								std::swap(sprites_[current_idx], sprites_[current_idx - 1]);
								sp_move_route.push_back(MoveRoute(MapIndex(row, col), MapIndex(row, col - 1)));
								continue;
							}
						}

						// 如果右边是空格并且空格上方没有精灵
						if (col + 1 < config_.width
							&& (config_.data[current_idx + 1])
							&& (!config_.data[previous_row_idx + 1])
							&& (sprites_[current_idx + 1] == NOSPRITE))
						{
							// 如果空格的右边是有效格
							if (col + 2 < config_.width && config_.data[current_idx + 2])
							{
								if (CalculateShortest(MapIndex(row, col)) <= CalculateShortest(MapIndex(row, col + 2)))
								{
									moved_set.insert(MapIndex(row, col + 1));
									std::swap(sprites_[current_idx], sprites_[current_idx + 1]);
									sp_move_route.push_back(MoveRoute(MapIndex(row, col), MapIndex(row, col + 1)));
									continue;
								}
								else if (sprites_[current_idx + 2] > NOSPRITE
										 && moved_set.find(MapIndex(row, col + 2)) == moved_set.end())
								{
									moved_set.insert(MapIndex(row, col + 1));
									std::swap(sprites_[current_idx + 2], sprites_[current_idx + 1]);
									sp_move_route.push_back(MoveRoute(MapIndex(row, col + 2), MapIndex(row, col + 1)));
									continue;
								}
							}
							else
							{
								moved_set.insert(MapIndex(row, col + 1));
								std::swap(sprites_[current_idx], sprites_[current_idx + 1]);
								sp_move_route.push_back(MoveRoute(MapIndex(row, col), MapIndex(row, col + 1)));
								continue;
							}
						}
					}
				}
			}
		}
	} while (moved_set.size() > before_size);

	// 记录下移动过的精灵索引
	for (auto &index : moved_set)
	{
		souch_scope_.update(index);
		moved_sprites_.insert(index);
	}

	// 通知界面播放移动动画
	int number = 0;
	const int total = added_set.size() + sp_move_route.size();
	for (auto &index : added_set)
	{
		moved_sprites_.insert(index);
		delegate_->OnSpriteFalldown(index, index, ++number, total);
	}
	for (auto &route : sp_move_route)
	{
		delegate_->OnSpriteFalldown(route.source, route.target, ++number, total);
	}

	return moved_set.empty() ? added_set.empty() == false : true;
}