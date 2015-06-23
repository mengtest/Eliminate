#include "GameLayer.h"

#include <queue>
#include <random>
#include "Config.h"
#include "Element.h"
#include "VisibleRect.h"
using namespace cocos2d;


GameLayer::GameLayer()
	: touch_lock_(false)
	, current_selected_(nullptr)
	, previous_selected_(nullptr)
{

}

GameLayer::~GameLayer()
{

}

bool GameLayer::init()
{
	if (!Layer::init())
	{
		return false;
	}

	// 节点批量渲染
	batch_node_ = SpriteBatchNode::create("elements.png");
	addChild(batch_node_);

	// 开启触摸
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(false);
	listener->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMoved, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	return true;
}

// 获取首行
int GameLayer::GetFirstLine()
{
	for (int idx = 0; idx < map_config_.width * map_config_.height; ++idx)
	{
		if (map_config_.data[idx])
		{
			return idx / map_config_.width;
		}
	}
	CCAssert(false, "invalid map!");
	return 0;
}

// 获取起点坐标
inline cocos2d::Vec2 GameLayer::GetStartPoint() const
{
	auto config = Config::GetInstance();
	return VisibleRect::center() - Vec2((config->GetElementWidth() * map_config_.width) / 2, -(config->GetElementHeight() * map_config_.height) / 2);
}

// 转换到全局坐标
Vec2 GameLayer::ConvertToPosition(const MapIndex &index) const
{
	Vec2 start_point = GetStartPoint();
	const int width = Config::GetInstance()->GetElementWidth();
	const int height = Config::GetInstance()->GetElementHeight();
	return Vec2(start_point.x + width / 2 + width * index.col, start_point.y - height / 2 - height * index.row);
}

// 转换到地图索引
MapIndex GameLayer::ConvertToMapIndex(const Vec2 &position) const
{
	Vec2 start_point = GetStartPoint();
	auto config = Config::GetInstance();
	for (int row = 0; row < map_config_.height; ++row)
	{
		for (int col = 0; col < map_config_.width; ++col)
		{
			float frist_col = start_point.x + col * config->GetElementWidth();
			float last_col = start_point.x + (col + 1) * config->GetElementWidth();
			float frist_row = start_point.y - row * config->GetElementHeight();
			float last_row = start_point.y - (row + 1) * config->GetElementHeight();
			if (position.x >= frist_col && position.x < last_col && position.y <= frist_row && position.y > last_row)
			{
				return MapIndex(row, col);
			}
		}
	}
	return MapIndex(INVALID_INDEX, INVALID_INDEX);
}

// 是否相邻
bool GameLayer::IsAdjacent(const MapIndex &a, const MapIndex &b)
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

// 随机元素类型
int GameLayer::RandElementType() const
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, map_config_.type);
	return dis(gen);
}

// 设置地图
void GameLayer::SetMap(const MapConfig &map_config)
{
	map_config_ = map_config;
	const unsigned int total = map_config_.height * map_config_.width;
	CCAssert(total == map_config_.data.size(), "Error map config");

	// 隐藏元素
	for (auto pair : all_elments)
	{
		free_elements.push_back(pair.second);
	}
	all_elments.clear();

	for (auto value : floor_elments)
	{
		value->setVisible(false);
	}

	// 初始搜索范围
	InitSouchScope();

	// 初始化变量
	int col = 0;
	int row = 0;
	char buffer[128];
	Vec2 start_point = GetStartPoint();
	const int width = Config::GetInstance()->GetElementWidth();
	const int height = Config::GetInstance()->GetElementHeight();

	// 创建地板元素
	for (unsigned int idx = 0; idx < total; ++idx)
	{
		row = idx / map_config_.width;
		col = idx % map_config_.width;
		if (idx >= floor_elments.size())
		{
			auto element = Sprite::createWithSpriteFrameName("gs_el_floor.png");
			batch_node_->addChild(element);
			floor_elments.push_back(element);
		}
		auto element = floor_elments[idx];
		element->setPosition(ConvertToPosition(MapIndex(row, col)));
		element->setVisible(map_config_.data[idx]);
	}

	// 根据地图数据创建元素
	for (unsigned int idx = 0; idx < total; ++idx)
	{
		if (map_config_.data[idx])
		{
			row = idx / map_config_.width;
			col = idx % map_config_.width;
			Element *element = nullptr;

			if (!free_elements.empty())
			{
				element = free_elements.back();
				free_elements.pop_back();
			}
			else
			{
				element = Element::createWithSpriteFrameName("gs_el_floor.png");
				batch_node_->addChild(element);
			}

			int type = RandElementType();
			element->SetType(type);
			element->SetIndex(MapIndex(row, col));
			all_elments.insert(std::make_pair(element->GetIndex(), element));

			sprintf(buffer, "gs_el_%02d.png", type);
			element->setPosition(ConvertToPosition(MapIndex(row, col)));
			element->setSpriteFrame(buffer);
		}
	}
}

// 获取可消除集合
std::set<MapIndex> GameLayer::GetCanEliminate(const MapIndex &index)
{
	auto element = all_elments.find(index);
	CCAssert(element != all_elments.end(), "");

	unsigned int lenght = 0;
	std::set<MapIndex> temp_set;
	std::set<MapIndex> return_set;
	const int type = (*element).second->GetType();
	const int width = Config::GetInstance()->GetElementWidth();
	const int height = Config::GetInstance()->GetElementHeight();

	// 横向遍历
	for (int col = 0; col < width; ++col)
	{
		auto itr = all_elments.find(MapIndex(index.row, col));
		if (itr != all_elments.end() && (*itr).second->GetType() == type)
		{
			++lenght;
			temp_set.insert(MapIndex(index.row, col));
		}
		else
		{
			if (lenght >= 3)
			{
				if (temp_set.find(index) != temp_set.end())
				{
					break;
				}
				else
				{
					temp_set.clear();
				}
			}
			lenght = 0;
			temp_set.clear();
		}
	}
	if (temp_set.size() < 3) temp_set.clear();
	lenght = 0;

	// 纵向遍历
	for (int row = 0; row < height; ++row)
	{
		auto itr = all_elments.find(MapIndex(row, index.col));
		if (itr != all_elments.end() && (*itr).second->GetType() == type)
		{
			++lenght;
			return_set.insert(MapIndex(row, index.col));
		}
		else
		{
			if (lenght >= 3)
			{
				if (return_set.find(index) != return_set.end())
				{
					break;
				}
				else
				{
					return_set.clear();
				}
			}
			lenght = 0;
			return_set.clear();

		}
	}
	if (return_set.size() < 3) return_set.clear();

	for (auto &value : temp_set)
	{
		return_set.insert(value);
	}

	return return_set;
}

// 获取可消除集合
std::set<MapIndex> GameLayer::GetToBeEliminated(Element *a, Element *b)
{
	CCAssert(a && b, "nullptr");

	auto eliminate_set = GetCanEliminate(a->GetIndex());
	for (auto value : GetCanEliminate(b->GetIndex()))
	{
		eliminate_set.insert(value);
	}
	return eliminate_set;
}

// 移动元素
void GameLayer::MoveElements(const MapIndex &source, const MapIndex &target)
{
	static int count = 0;

	// 回调函数
	auto _fill_callback_ = [&]()
	{
		if (--count == 0)
		{
			FillElements();
		}
	};

	// 源元素和目标元素交换属性
	auto itr = all_elments.find(source);
	(*itr).second->SetIndex(target);
	(*itr).second->Fill(Config::GetInstance()->GetElementFillTime(), ConvertToPosition(target), _fill_callback_);
	all_elments[target] = (*itr).second;
	all_elments.erase(itr);

	CalculSouchScope(source);
	CalculSouchScope(target);
	moved_elements_.insert(target);
	++count;
}

// 计算最短距离
int GameLayer::ShortestDistance(const MapIndex &index)
{
	// 求最短距离
	std::vector<int> heap;
	for (int col = 0; col < map_config_.width; ++col)
	{
		if (map_config_.data[col])
		{
			a_star::AStarParam param;
			param.start_point.row = 0;
			param.start_point.col = col;
			param.end_point.row = index.row;
			param.end_point.col = index.col;
			param.total_row = map_config_.height;
			param.total_col = map_config_.width;
			param.is_can_reach = [&](const a_star::Vec2 &point)
			{
				return map_config_.data[point.row * map_config_.width + point.col];
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

// 初始化搜索范围
void GameLayer::InitSouchScope()
{
	for (unsigned int i = 0; i < search_scope_.size(); ++i)
	{
		search_scope_[i] = -1;
	}
}

// 计算搜索范围
inline void GameLayer::CalculSouchScope(const MapIndex &index)
{
	search_scope_[0] = search_scope_[0] == -1 ? index.row : index.row < search_scope_[0] ? index.row : search_scope_[0];
	search_scope_[1] = search_scope_[1] == -1 ? index.col : index.col < search_scope_[1] ? index.col : search_scope_[1];
	search_scope_[2] = search_scope_[2] == -1 ? index.row : index.row > search_scope_[2] ? index.row : search_scope_[2];
	search_scope_[3] = search_scope_[3] == -1 ? index.col : index.col > search_scope_[3] ? index.col : search_scope_[3];
}

// 填充首行元素
void GameLayer::FillFristLineElements()
{
	char buffer[128];
	Vec2 start_point = GetStartPoint();
	const int frist_line = GetFirstLine();
	const int width = Config::GetInstance()->GetElementWidth();
	const int height = Config::GetInstance()->GetElementHeight();

	for (int col = 0; col < map_config_.width; ++col)
	{
		const int idx = frist_line * map_config_.width + col;
		if (map_config_.data[idx] && (all_elments.find(MapIndex(frist_line, col)) == all_elments.end()))
		{
			Element *element = nullptr;
			int type = RandElementType();
			sprintf(buffer, "gs_el_%02d.png", type);

			if (free_elements.empty())
			{
				element = Element::createWithSpriteFrameName("gs_el_floor.png");
				batch_node_->addChild(element);
			}
			else
			{
				element = free_elements.back();
				element->setVisible(true);
				free_elements.pop_back();
			}

			// 跳转层级
			element->setLocalZOrder(1);
			element->setLocalZOrder(0);

			// 设置属性
			element->SetType(type);
			element->SetIndex(MapIndex(frist_line, col));
			element->setSpriteFrame(buffer);
			element->setPosition(ConvertToPosition(MapIndex(frist_line, col)));
			moved_elements_.insert(element->GetIndex());
			all_elments.insert(std::make_pair(element->GetIndex(), element));
		}
	}
}

// 填充元素
void GameLayer::FillElements()
{
	// 补充第一行元素
	FillFristLineElements();

	// 向下填充元素
	unsigned int before_size = 0;
	std::set<MapIndex> moved_set;
	const int frist_line = GetFirstLine();
	do
	{
		before_size = moved_set.size();

		// 跳转搜索范围
		if (search_scope_[0] > 0) --search_scope_[0];
		if (search_scope_[1] > 0) --search_scope_[1];
		if (search_scope_[2] < map_config_.height - 1) ++search_scope_[2];
		if (search_scope_[3] < map_config_.width - 1) ++search_scope_[3];

		for (int row = search_scope_[0]; row <= search_scope_[2]; ++row)
		{
			for (int col = search_scope_[1]; col <= search_scope_[3]; ++col)
			{
				// 如果此处有元素并且在此轮中没有被移动过
				if ((all_elments.find(MapIndex(row, col)) != all_elments.end()) && (moved_set.find(MapIndex(row, col)) == moved_set.end()))
				{
					// 向下补充
					if ((row + 1 < map_config_.height)
						&& (map_config_.data[(row + 1) * map_config_.width + col])
						&& (all_elments.find(MapIndex(row + 1, col)) == all_elments.end()))
					{
						MoveElements(MapIndex(row, col), MapIndex(row + 1, col));
						moved_set.insert(MapIndex(row + 1, col));
					}
					// 横向移动
					else if (row > frist_line)
					{
						// 如果左边是空格并且空格上方没有元素
						if (col - 1 >= 0
							&& (map_config_.data[row * map_config_.width + col - 1])
							&& (!map_config_.data[(row - 1) * map_config_.width + col - 1])
							&& (all_elments.find(MapIndex(row, col - 1)) == all_elments.end()))
						{
							// 如果空格的左边是有效格
							if (col - 2 >= 0 && map_config_.data[row * map_config_.width + col - 2])
							{
								if (ShortestDistance(MapIndex(row, col)) <= ShortestDistance(MapIndex(row, col - 2)))
								{
									MoveElements(MapIndex(row, col), MapIndex(row, col - 1));
									moved_set.insert(MapIndex(row, col - 1));
									continue;
								}
								else if ((all_elments.find(MapIndex(row, col - 2)) != all_elments.end())
										 && moved_set.find(MapIndex(row, col - 2)) == moved_set.end())
								{
									MoveElements(MapIndex(row, col - 2), MapIndex(row, col - 1));
									moved_set.insert(MapIndex(row, col - 1));
									continue;
								}
							}
							else
							{
								MoveElements(MapIndex(row, col), MapIndex(row, col - 1));
								moved_set.insert(MapIndex(row, col - 1));
								continue;
							}
						}

						// 如果右边是空格并且空格上方没有元素
						if (col + 1 < map_config_.width
							&& (map_config_.data[row * map_config_.width + col + 1])
							&& (!map_config_.data[(row - 1) * map_config_.width + col + 1])
							&& (all_elments.find(MapIndex(row, col + 1)) == all_elments.end()))
						{
							// 如果空格的右边是有效格
							if (col + 2 < map_config_.width && map_config_.data[row * map_config_.width + col + 2])
							{
								if (ShortestDistance(MapIndex(row, col)) <= ShortestDistance(MapIndex(row, col + 2)))
								{
									MoveElements(MapIndex(row, col), MapIndex(row, col + 1));
									moved_set.insert(MapIndex(row, col + 1));
									continue;
								}
								else if ((all_elments.find(MapIndex(row, col + 2)) != all_elments.end())
										 && moved_set.find(MapIndex(row, col + 2)) == moved_set.end())
								{
									MoveElements(MapIndex(row, col + 2), MapIndex(row, col + 1));
									moved_set.insert(MapIndex(row, col + 1));
									continue;
								}
							}
							else
							{
								MoveElements(MapIndex(row, col), MapIndex(row, col + 1));
								moved_set.insert(MapIndex(row, col + 1));
								continue;
							}
						}
					}
				}
			}
		}
	} while (moved_set.size() > before_size);

	// 补充完毕
	if (moved_set.empty())
	{
		std::set<MapIndex> eliminate_set;
		for (auto value : moved_elements_)
		{
			if (all_elments.find(value) != all_elments.end())
			{
				for (auto idx : GetCanEliminate(value))
				{
					eliminate_set.insert(idx);
				}
			}		
		}
		moved_elements_.clear();
		if (!eliminate_set.empty())
		{
			DoEliminate(eliminate_set);
		}
		else
		{
			// 初始搜索范围
			InitSouchScope();

			// 允许触摸事件
			touch_lock_ = false;
		}
	}
}

// 执行消除
void GameLayer::DoEliminate(const std::set<MapIndex> &elements)
{
	static int count = 0;

	// 消除回调
	auto _eliminate_callback_ = [&]()
	{
		if (--count == 0)
		{
			FillElements();
		}
	};

	// 消除元素
	for (auto index : elements)
	{
		auto itr = all_elments.find(index);
		CCAssert(itr != all_elments.end(), "error");

		// 计算搜索范围
		CalculSouchScope(index);

		// 执行消除
		++count;
		(*itr).second->Eliminate(_eliminate_callback_);
		free_elements.push_back((*itr).second);
		all_elments.erase(itr);
	}
}

// 完成位置交换
void GameLayer::SwapElementPositionFinished()
{
	// 判断语句将在不可消除元素时生效
	if (!current_selected_ && !previous_selected_)
	{
		touch_lock_ = false;
		return;
	}

	// 如果类型相同
	if (current_selected_->GetType() == previous_selected_->GetType())
	{
		SwapElementPosition(current_selected_, previous_selected_);
	}
	else
	{
		// 获取将被消除的元素索引
		auto to_be_eliminated = GetToBeEliminated(current_selected_, previous_selected_);

		// 如果不存在可消除的元素则把选中的两个元素位置还原
		if (!to_be_eliminated.empty())
		{
			DoEliminate(to_be_eliminated);
		}
		else
		{
			SwapElementPosition(current_selected_, previous_selected_);
		}
	}

	current_selected_ = nullptr;
	previous_selected_ = nullptr;
}

// 交换元素位置
void GameLayer::SwapElementPosition(Element *a, Element *b)
{
	CCAssert(a && b, "nullptr");

	// 回调函数
	auto _swap_callback_ = [=]()
	{
		static int count = 0;
		if (++count == 2)
		{
			count = 0;
			MapIndex index = a->GetIndex();
			a->SetIndex(b->GetIndex());
			b->SetIndex(index);
			std::swap(all_elments[a->GetIndex()], all_elments[b->GetIndex()]);
			SwapElementPositionFinished();
		}
	};

	// 互换位置
	auto config = Config::GetInstance();
	a->Move(config->GetElementMoveTime(), b->getPosition(), _swap_callback_);
	b->Move(config->GetElementMoveTime(), a->getPosition(), _swap_callback_);
}

// 点击事件
bool GameLayer::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event)
{
	return true;
}

// 移动事件
void GameLayer::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event)
{
	if (touch_lock_ || all_elments.empty()) return;

	MapIndex index = ConvertToMapIndex(touch->getLocation());
	auto itr = all_elments.find(index);
	if (itr != all_elments.end())
	{
		Element *current = (*itr).second;
		if (previous_selected_)
		{
			if (previous_selected_ != current)
			{
				if (IsAdjacent(current->GetIndex(), previous_selected_->GetIndex()))
				{
					touch_lock_ = true;
					current_selected_ = current;
					SwapElementPosition(previous_selected_, current_selected_);
				}
			}
		}
		else
		{
			previous_selected_ = current;
		}
	}
	else
	{
		previous_selected_ = nullptr;
	}
}