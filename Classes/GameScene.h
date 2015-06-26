/**
 * 游戏场景
 * author: zhangpanyi@live.com
 * https://github.com/zhangpanyi/Eliminate
 */

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