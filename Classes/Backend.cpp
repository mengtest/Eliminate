#include "Backend.h"
#include <random>


Backend::Backend()
	: initialized_(false)
{

}

void Backend::Clear()
{
	elements_.clear();
	config_.type = 0;
	config_.width = 0;
	config_.height = 0;
	config_.data.clear();
	initialized_ = false;
}

// 设置地图
void Backend::SetMap(const MapConfig &config)
{
	if (config.data.size() == config.width * config.height)
	{
		config_ = config;
		elements_.clear();
		ReGeneration();
		souch_scope_.init();
		initialized_ = true;
	}
	else
	{
		throw std::runtime_error("Invalid map configuration!");
	}
}

// 随机元素类型
int Backend::RandElementType() const
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, config_.type);
	return dis(gen);
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
		elements_.push_back(config_.data[idx] ? RandElementType() : 0);
	}
}

// 有效元素索引
bool Backend::IsValidElements(const MapIndex &index)
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	if (index.col >= 0 && index.row >= 0 && index.col < config_.width && index.row < config_.height)
	{
		return elements_[index.row * config_.width + index.col] != 0;
	}
	return false;
}

// 遍历地图
void Backend::VisitMap(const VisitCallback &step)
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	for (int idx = 0; idx < config_.width * config_.height; ++idx)
	{
		step(MapIndex(idx / config_.width, idx % config_.width), elements_[idx]);
	}
}

// 交换元素
void Backend::SwapElement(const MapIndex &a, const MapIndex &b)
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	if (!IsValidElements(a) || !IsValidElements(b))
	{
		throw std::runtime_error("invalid element index!");
	}
	std::swap(elements_[a.row * config_.width + a.col], elements_[b.row * config_.width + b.col]);
}

// 是否可消除
bool Backend::IsCanEliminate(const MapIndex &index, std::set<MapIndex> &out)
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	out.clear();
	if (IsValidElements(index))
	{
		std::set<MapIndex> elements_set;
		const int type = elements_[index.row * config_.width + index.col];

		// 横向遍历
		for (int col = 0; col < config_.width; ++col)
		{
			const int idx = index.row * config_.width + col;
			if (elements_[idx] == type)
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
			if (elements_[idx] == type)
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

// 执行消除
void Backend::DoEliminate(std::set<MapIndex> &in_elements, const EliminateCallback &step)
{
	if (!initialized_)
	{
		throw std::runtime_error("map configuration is not set!");
	}

	for (auto &index : in_elements)
	{
#ifdef _DEBUG
		if (!IsValidElements(index))
		{
			throw std::runtime_error("invalid element index!");
		}
#endif
		elements_[index.row * config_.width + index.col] = 0;
		souch_scope_.update(index);
		step(index);
	}
}