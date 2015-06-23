#include "GameScene.h"

#include <sstream>
#include "Config.h"
#include "GameLayer.h"
#include "VisibleRect.h"
using namespace cocos2d;


GameScene::GameScene()
{

}

GameScene::~GameScene()
{

}

bool GameScene::init()
{
	if (!Scene::init())
	{
		return false;
	}

	// 加载纹理
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("elements.plist");

	// 创建背景
	auto background_ = Sprite::create("background.png");
	background_->setPosition(Vec2(VisibleRect::center().x, VisibleRect::bottom().y + background_->getContentSize().height / 2));
	addChild(background_);

	// 创建游戏图层
	auto layer = GameLayer::create();
	layer->SetMap(Config::GetInstance()->ReadMapConfig("map/map.tmx"));
	addChild(layer);

	return true;
}