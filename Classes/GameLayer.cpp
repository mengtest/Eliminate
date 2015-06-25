#include "GameLayer.h"

#include "Config.h"
#include "Element.h"
#include "VisibleRect.h"
using namespace cocos2d;


GameLayer::GameLayer()
	: touch_lock_(false)
	, backend_(this)
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

// 初始化地板
void GameLayer::InitFloor()
{
	for (auto floor_ptr : floor_elments)
	{
		floor_ptr->setVisible(false);
	}
}

// 初始化元素
void GameLayer::InitElements()
{
	for (auto pair : used_elments)
	{
		pair.second->stopAllActions();
		pair.second->setScale(1.0f);
		pair.second->setOpacity(255);
		pair.second->setVisible(false);
		free_elements.push_back(pair.second);
	}
	used_elments.clear();
}

// 获取起点坐标
inline cocos2d::Vec2 GameLayer::GetStartPoint() const
{
	auto config = Config::GetInstance();
	return VisibleRect::center() - Vec2((config->GetElementWidth() * backend_.GetMapWidth()) / 2, -(config->GetElementHeight() * backend_.GetMapHeight()) / 2);
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
	for (int row = 0; row < backend_.GetMapHeight(); ++row)
	{
		for (int col = 0; col < backend_.GetMapWidth(); ++col)
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

// 完成消除
void GameLayer::OnEliminateFinished()
{
	// 更新界面
	InitElements();
	backend_.VisitMap();

	// 精灵自动移动
	if (!backend_.AutoMoveSprite())
	{
		std::set<MapIndex> eliminate_set;
		if (!backend_.MovedSpriteCanEliminate(eliminate_set))
		{
			touch_lock_ = false;
			previous_selected_.col = INVALID_INDEX;
			previous_selected_.row = INVALID_INDEX;
		}
		else
		{
			backend_.DoEliminate(eliminate_set);		
		}
	}
}

// 消除元素事件
void GameLayer::OnEliminate(const MapIndex &index, unsigned int number, unsigned int total)
{
	// 执行ui上的消除
	auto element_ptr = dynamic_cast<Element *>((*this->used_elments.find(index)).second);
	element_ptr->Eliminate([=]()
	{
		if (number == total)
		{
			OnEliminateFinished();
		}
	});
}

// 刷新地图事件
void GameLayer::OnRefreshMap(const MapIndex &index, int type)
{
	// 更新地板
	size_t idx = index.row * backend_.GetMapWidth() + index.col;
	if (idx >= floor_elments.size())
	{
		auto element = Sprite::createWithSpriteFrameName("gs_el_floor.png");
		batch_node_->addChild(element);
		floor_elments.push_back(element);
	}
	auto element = floor_elments[idx];
	element->setPosition(ConvertToPosition(index));
	element->setVisible(type != Backend::NOTHING);
	element->setLocalZOrder(0);

	// 更新元素
	if (type > 0)
	{
		char buffer[128];
		Sprite *element = nullptr;
		sprintf(buffer, "gs_el_%02d.png", type);

		auto itr = used_elments.find(index);
		if (itr != used_elments.end())
		{
			element = (*itr).second;
		}
		else
		{
			if (!free_elements.empty())
			{
				element = free_elements.back();
				element->setSpriteFrame(buffer);
				free_elements.pop_back();
			}
			else
			{
				element = Element::createWithSpriteFrameName(buffer);
				batch_node_->addChild(element);
			}
		}
		element->setVisible(true);
		element->setLocalZOrder(1);
		element->setPosition(ConvertToPosition(index));
		used_elments.insert(std::make_pair(index, element));
	}
}

// 补充精灵事件
void GameLayer::OnMoveSprite(const MapIndex &source, const MapIndex &target, unsigned int number, unsigned int total)
{
	// 执行ui上的补充动画
	auto config = Config::GetInstance();
	auto itr = used_elments.find(source);
	if (itr != used_elments.end())
	{
		auto source_ptr = dynamic_cast<Element *>((*itr).second);
		used_elments.erase(itr);
		used_elments[target] = source_ptr;

		if (source == target)
		{
			source_ptr->setPosition(ConvertToPosition(target));
			if (number == total) OnEliminateFinished();
		}
		else
		{
			source_ptr->Fill(source == target ? 0.0f : config->GetElementFillTime(), ConvertToPosition(target), [=]()
			{
				if (number == total) OnEliminateFinished();
			});
		}
	}
	else
	{
		CCASSERT(false, "");
	}
}

// 设置地图
void GameLayer::SetMap(const MapConfig &map_config)
{
	backend_.SetMap(map_config);

	InitFloor();
	InitElements();
	backend_.VisitMap();
}

// 完成位置交换
void GameLayer::SwapElementPositionFinished()
{
	// 重置标志
	static bool reset = false;

	// 是否重置
	if (reset)
	{
		reset = false;
		touch_lock_ = false;
		previous_selected_.col = INVALID_INDEX;
		previous_selected_.row = INVALID_INDEX;
		return;
	}

	// 如果类型相同
	std::set<MapIndex> eliminate_set;
	if (!backend_.IsCanEliminate(current_selected_, previous_selected_, eliminate_set))
	{
		reset = true;
		SwapElementPosition();
	}
	else
	{
		// 执行消除
		backend_.DoEliminate(eliminate_set);
	}
}

// 交换元素位置
void GameLayer::SwapElementPosition()
{
	CCAssert(previous_selected_ && current_selected_, "INVALID_INDEX");

	// 逻辑上更换位置
	backend_.SwapSprite(previous_selected_, current_selected_);
	auto current_ptr = dynamic_cast<Element *>((*used_elments.find(current_selected_)).second);
	auto previous_ptr = dynamic_cast<Element *>((*used_elments.find(previous_selected_)).second);

	// 回调函数
	auto swap_callback = [=]()
	{
		static int count = 0;
		if (++count == 2)
		{
			count = 0;
			std::swap(used_elments[current_selected_], used_elments[previous_selected_]);
			SwapElementPositionFinished();
		}
	};

	
	// ui上更换位置
	auto config = Config::GetInstance();
	current_ptr->Move(config->GetElementMoveTime(), previous_ptr->getPosition(), swap_callback);
	previous_ptr->Move(config->GetElementMoveTime(), current_ptr->getPosition(), swap_callback);
}

// 点击事件
bool GameLayer::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event)
{
	return true;
}

// 移动事件
void GameLayer::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event)
{
	if (touch_lock_ || used_elments.empty()) return;

	MapIndex index = ConvertToMapIndex(touch->getLocation());
	auto itr = used_elments.find(index);
	if (itr != used_elments.end())
	{
		if (previous_selected_)
		{
			if (previous_selected_ != (*itr).first)
			{
				if (backend_.IsAdjacent((*itr).first, previous_selected_))
				{
					touch_lock_ = true;
					current_selected_ = (*itr).first;
					SwapElementPosition();
				}
			}
		}
		else
		{
			previous_selected_ = (*itr).first;
		}
	}
	else
	{
		previous_selected_.col = INVALID_INDEX;
		previous_selected_.row = INVALID_INDEX;
	}
}