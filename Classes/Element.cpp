#include "Element.h"
using namespace cocos2d;

Element* Element::create()
{
	Element *sprite = new (std::nothrow) Element();
	if (sprite && sprite->init())
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

Element* Element::create(const std::string& filename)
{
	Element *sprite = new (std::nothrow) Element();
	if (sprite && sprite->initWithFile(filename))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

Element* Element::create(const std::string& filename, const Rect& rect)
{
	Element *sprite = new (std::nothrow) Element();
	if (sprite && sprite->initWithFile(filename, rect))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

Element* Element::createWithTexture(Texture2D *texture)
{
	Element *sprite = new (std::nothrow) Element();
	if (sprite && sprite->initWithTexture(texture))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

Element* Element::createWithTexture(Texture2D *texture, const Rect& rect, bool rotated)
{
	Element *sprite = new (std::nothrow) Element();
	if (sprite && sprite->initWithTexture(texture, rect, rotated))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

Element* Element::createWithSpriteFrame(SpriteFrame *spriteFrame)
{
	Element *sprite = new (std::nothrow) Element();
	if (sprite && spriteFrame && sprite->initWithSpriteFrame(spriteFrame))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

Element* Element::createWithSpriteFrameName(const std::string& spriteFrameName)
{
	SpriteFrame *frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteFrameName);

#if COCOS2D_DEBUG > 0
	char msg[256] = { 0 };
	sprintf(msg, "Invalid spriteFrameName: %s", spriteFrameName.c_str());
	CCASSERT(frame != nullptr, msg);
#endif

	return createWithSpriteFrame(frame);
}

void Element::Reset()
{
	setScale(1.0f);
	setOpacity(255);
	stopAllActions();
}

void Element::Move(float duration, const cocos2d::Vec2& position, const std::function<void()> &func)
{
	CCAssert(func, "Invalid function object");
	runAction(Sequence::create(MoveTo::create(duration, position), CallFunc::create(func), nullptr));
}

void Element::Falldown(float duration, const cocos2d::Vec2& position, const std::function<void()> &func)
{
	CCAssert(func, "Invalid function object");
	Move(duration, position, func);
}

void Element::Eliminate(const std::function<void()> &func)
{
	CCAssert(func, "Invalid function object");
	runAction(Sequence::create(EaseExponentialIn::create(ScaleTo::create(0.4f, 0.5f)),
		EaseExponentialOut::create(ScaleTo::create(0.4f, 1.0f)),
		FadeOut::create(0.1f),
		CallFunc::create([=](){ setOpacity(255); setScale(1.0f); setVisible(false); }),
		CallFunc::create(func),
		nullptr));
}