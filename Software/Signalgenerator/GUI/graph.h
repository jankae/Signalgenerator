#ifndef GRAPH_H_
#define GRAPH_H_

#include "widget.h"
#include "display.h"

class Graph : public Widget {
public:
	Graph(const int32_t *values, uint16_t num, uint16_t height, color_t color, const unit_t *unit);

	void newColor(color_t color);
	void newData(const int32_t *data);
private:
	void draw(coords_t offset) override;

	Widget::Type getType() override { return Widget::Type::Graph; };

	static constexpr color_t Background = COLOR_BG_DEFAULT;
	static constexpr color_t Border = COLOR_FG_DEFAULT;

    const int32_t *values;
    color_t color;
    const unit_t *unit;
};

#endif
