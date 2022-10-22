#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <cmath>

namespace svg {
    
//namespace color {

struct Rgb {
    Rgb() = default;
    Rgb(unsigned int r, unsigned int g, unsigned int b) : red{static_cast<uint8_t>(r)},
    green{static_cast<uint8_t>(g)}, blue{static_cast<uint8_t>(b)} {
    }
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};
    
std::ostream& operator<<(std::ostream& ost, Rgb rgb);
    
struct Rgba {
    Rgba() = default;
    Rgba(unsigned int r, unsigned int g, unsigned int b, double o) : red{static_cast<uint8_t>(r)},
    green{static_cast<uint8_t>(g)}, blue{static_cast<uint8_t>(b)}, opacity{o} {
    }
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};
    
std::ostream& operator<<(std::ostream& ost, Rgba rgba);
    
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

inline const Color NoneColor{};
    
using namespace std::literals; 
    
struct ColorPrinter {  
    std::ostream& ost;
    
    void operator()(std::monostate) const {
        ost << "none"sv;
    }
    void operator()(const std::string& color) const {
        ost << color;
    }
    void operator()(Rgb rgb) const {
        ost << rgb;
    }
    void operator()(Rgba rgba) const {
        ost << rgba;
    }
};
    
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
    
std::ostream& operator<<(std::ostream& ost, StrokeLineCap cap);
std::ostream& operator<<(std::ostream& ost, StrokeLineJoin join);
std::ostream& operator<<(std::ostream& ost, Color color);
    
const std::vector<std::string> line_cap{"butt", "round", "square"};
const std::vector<std::string> line_join{"arcs", "bevel", "miter", "miter-clip", "round"};
    
template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_line_cap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_line_join_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv;
            std::visit(ColorPrinter{out}, *fill_color_);
            out << "\""sv;
        }
        
        if (stroke_color_) {
            out << " stroke=\""sv;
            std::visit(ColorPrinter{out}, *stroke_color_);
            out << "\""sv;
        }
        
        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (stroke_line_cap_) {
            out << " stroke-linecap=\""sv << CapToString(*stroke_line_cap_) << "\""sv;
        }
        if (stroke_line_join_) {
            out << " stroke-linejoin=\""sv << JoinToString(*stroke_line_join_) << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }
    
    std::string CapToString(StrokeLineCap cap) const {
        return line_cap[static_cast<int>(cap)];
    }
    std::string JoinToString(StrokeLineJoin join) const {
        return line_join[static_cast<int>(join)];
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    //Color fill_color_;
    //Color stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};
    
    
//} // namespace color

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

class Circle final : public Object, public /*color::*/PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

class Polyline : public Object, public /*color::*/PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    
    std::vector<Point> points_;
};

class Text : public Object, public /*color::*/PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;
    
    std::string TextParse() const {
        std::string parse_txt;
        for (size_t i = 0; i < data_.size(); ++i) {
            switch(data_[i]) {
            case '\"':
                parse_txt += "&quot;";
                break;
            case '<':
                parse_txt += "&lt;";
                break;
            case '>':
                parse_txt += "&gt;";
                break;
            case '\'':
                parse_txt += "&apos;";
                break;
            case '&':
                parse_txt += "&amp;";
                break;
            default:
                parse_txt += data_[i];
                break;
            }
        }
        return parse_txt;
    }
    
    Point position_;
    Point offset_;
    uint32_t size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};
    
class ObjectContainer {
public:
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    
    template <typename Object>
    void Add(Object obj) {
        AddPtr(std::make_unique<Object>(std::move(obj)));
    }
    
    virtual ~ObjectContainer() = default;
};

class Document : public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;
    
    void Render(std::ostream& out) const;
private:
    std::vector<std::unique_ptr<Object>> objects_;
};
    
class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    
    virtual ~Drawable() = default;
}; 
    

}  // namespace svg