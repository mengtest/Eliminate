#include "Config.h"
#include "cocos2d.h"
#include "json/document.h"
#include "2d/CCTMXXMLParser.h"
using namespace cocos2d;


Config::Config()
	: fall_down_time_(0.0f)
	, move_time_(0.0f)
	, element_width_(0)
	, element_height_(0)
	, type_quantity_(0)
{
	ReadConfigFile();
}

Config::~Config()
{
}

/* 读取配置文件 */
void Config::ReadConfigFile()
{
	Data data = FileUtils::getInstance()->getDataFromFile("config/config.json");
	CCAssert(!data.isNull(), "The config/config.json file does not exist");

	rapidjson::Document doc;
	std::string json((char *)data.getBytes(), data.getSize());
	doc.Parse<0>(json.c_str());
	CCAssert(!doc.HasParseError(), "doc.HasParseError()");

	element_width_ = doc["Width"].GetInt();
	element_height_ = doc["Height"].GetInt();
	type_quantity_ = doc["TypeQuantity"].GetInt();
	move_time_ = doc["MoveTime"].GetDouble();
	fall_down_time_ = doc["FallDownTime"].GetDouble();
}

/* 读取地图配置文件 */
MapConfig Config::ReadMapConfig(const std::string &filename)
{
	MapConfig config;

	Size map_size;
	auto map_info = TMXMapInfo::create(filename);
	map_info->setTileSize(map_size);

	auto layers = map_info->getLayers();
	CCAssert(!layers.empty(), "");

	auto tiles = layers.front()->_tiles;
	auto layer_ize = layers.front()->_layerSize;

	config.width = layer_ize.width;
	config.height = layer_ize.height;
	config.type_quantity = atoi(layers.front()->_name.c_str());
	const size_t max_size = layer_ize.width * layer_ize.height;

	if (config.type_quantity > type_quantity_)
	{
		config.type_quantity = type_quantity_;
	}

	for (size_t idx = 0; idx < max_size; ++idx)
	{
		config.data.push_back(tiles[idx] != 0);
	}

	return config;
}