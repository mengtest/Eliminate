/**************************************************
游戏元素
**************************************************/

#pragma once

#include "Types.h"
#include "cocos2d.h"

class Element final : public cocos2d::Sprite
{
public:
	/* 创建元素的工厂函数 */
	static Element* create();
	static Element* create(const std::string& filename);
	static Element* create(const std::string& filename, const cocos2d::Rect& rect);
	static Element* createWithTexture(cocos2d::Texture2D *texture);
	static Element* createWithTexture(cocos2d::Texture2D *texture, const cocos2d::Rect& rect, bool rotated = false);
	static Element* createWithSpriteFrame(cocos2d::SpriteFrame *spriteFrame);
	static Element* createWithSpriteFrameName(const std::string& spriteFrameName);

public:
	/**
	 * 移动
	 * @param duration 耗时
	 * @param position 目标位置
	 * @param func 回调函数
	 */
	void Move(float duration, const cocos2d::Vec2& position, const std::function<void()> &func);

	/**
	 * 填充
	 * @param duration 耗时
	 * @param position 目标位置
	 * @param func 回调函数
	 */
	void Fill(float duration, const cocos2d::Vec2& position, const std::function<void()> &func);

	/**
	 * 消除
	 * @param func 回调函数
	 */
	void Eliminate(const std::function<void()> &func);

public:
	/**
	 * 获取类型
	 */
	int GetType() const
	{
		return type_;
	}

	/**
	 * 设置类型
	 */
	void SetType(int type)
	{
		type_ = type;
	}

	/**
	 * 获取索引
	 */
	MapIndex GetIndex() const
	{
		return map_index_;
	}

	/**
	 * 设置索引
	 */
	void SetIndex(const MapIndex &index)
	{
		map_index_ = index;
	}

private:
	Element() = default;
	~Element() = default;

private:
	int			type_;
	MapIndex	map_index_;
};