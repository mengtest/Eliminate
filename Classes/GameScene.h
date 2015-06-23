/**************************************************
游戏场景
**************************************************/

#pragma once

#include "cocos2d.h"

class GameScene final : public cocos2d::Scene
{
public:
	GameScene();
	~GameScene();

	virtual bool init() override;

	CREATE_FUNC(GameScene);
};