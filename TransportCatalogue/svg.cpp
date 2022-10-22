#include "svg.h"

#include <iomanip>

#include "svg.h"

namespace svg {

using namespace std::literals;
    
std::ostream& operator<<(std::ostream& ost, StrokeLineCap cap) {
	ost << line_cap[static_cast<int>(cap)];
	return ost;
}
    
std::ostream& operator<<(std::ostream& ost, StrokeLineJoin join) {
    ost << line_join[static_cast<int>(join)];
    return ost;
}
    
std::ostream& operator<<(std::ostream& ost, Rgb rgb) {
    ost << "rgb("sv << static_cast<int>(rgb.red) << ',' << static_cast<int>(rgb.green) << ',' << static_cast<int>(rgb.blue) << ')';
    return ost;
}
    
std::ostream& operator<<(std::ostream& ost, Rgba rgba) {
    ost << "rgba("sv << static_cast<int>(rgba.red) << ',' << static_cast<int>(rgba.green) << ',' << static_cast<int>(rgba.blue) << ',' << rgba.opacity << ')';
    return ost;
}
    
std::ostream& operator<<(std::ostream& ost, Color color) {
    std::visit(ColorPrinter{ost}, color); 
    return ost;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}
    
// ---------- Polyline ------------------
    
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}
    
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    if (points_.size()) {
        size_t i = 0;
        for (const auto& point : points_) {
            out << point.x << ',' << point.y;// << ' ';
            if (i++ != points_.size()-1) {
                out << ' ';
            }
        }
    }
    else {
        out << "";
    }
    out << "\"";
    RenderAttrs(context.out);
    out << "/>"sv;
}
    
// ---------- Text ------------------
    
Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}
    
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}
    
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}
    
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}
    
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}
    
Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}
    
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    RenderAttrs(context.out);
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\"" << size_ << "\"";
    if (font_family_.size()) {
        out << " font-family=\"" << font_family_ << "\"";
    }
    if (font_weight_.size()) {
        out << " font-weight=\"" << font_weight_ << "\"";
    }
    //RenderAttrs(context.out);
    out << '>' << TextParse() << "</text>";
}
    
// ---------- Document ------------------
    
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(move(obj));
}
    
void Document::Render(std::ostream& out) const {
    RenderContext ren(out);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"<< std::endl;
    for (auto& obj : objects_) {
        obj->Render(ren);
    }
    out << "</svg>";
}

}  // namespace svg