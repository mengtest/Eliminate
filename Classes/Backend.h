#pragma once

#include <set>
#include <cstdint>
#include <functional>
#include "Types.h"

class Backend
{
	struct Scope
	{
		int min_row;
		int min_col;
		int max_row;
		int max_col;

		void init()
		{
			min_row = min_col = max_row = max_col = -1;
		}

		void update(const MapIndex &index)
		{
			min_row = min_row == -1 ? index.row : index.row < min_row ? index.row : min_row;
			min_col = min_col == -1 ? index.col : index.col < min_col ? index.col : min_col;
			max_row = max_row == -1 ? index.row : index.row > max_row ? index.row : max_row;
			max_col = max_col == -1 ? index.col : index.col > max_col ? index.col : max_col;
		}
	};

	typedef std::function<void(const MapIndex &index)> EliminateCallback;
	typedef std::function<void(const MapIndex &index, int type)> VisitCallback;

public:
	Backend();
	~Backend() = default;

public:
	void Clear();

	/**
	 * 重新生成地图
	 */
	void ReGeneration();

	/**
	 * 设置地图
	 * @param config 地图配置
	 */
	void SetMap(const MapConfig &config);

	/**
	 * 有效元素索引
	 * @param 元素索引
	 */
	bool IsValidElements(const MapIndex &index);

	/**
	 * 遍历地图
	 * @param step 回调函数
	 * VisitCallback type=0表示此索引是地板
	 */
	void VisitMap(const VisitCallback &step);

	/**
	 * 交换元素
	 * @param a 元素a索引
	 * @param b 元素b索引
	 */
	void SwapElement(const MapIndex &a, const MapIndex &b);

public:
	/**
	 * 是否可消除
	 * @param index 焦点索引
	 * @param out 可消除索引集合
	 * @return bool
	 */
	virtual bool IsCanEliminate(const MapIndex &index, std::set<MapIndex> &out);

	/**
	 * 执行消除
	 * @param 将被消除的元素集合
	 */
	virtual void DoEliminate(std::set<MapIndex> &in_elements, const EliminateCallback &step);

	/**
	 * 补充元素(用于执行完所有消除之后调用)
	 */
	virtual void SuppliesElements();

private:
	/**
	 * 随机元素类型
	 */
	int RandElementType() const;

private:
	bool initialized_;
	MapConfig config_;
	Scope souch_scope_;
	std::vector<int> elements_;
};