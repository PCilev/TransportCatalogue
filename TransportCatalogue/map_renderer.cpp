#include "map_renderer.h"

namespace renderer {
    
using namespace std;
    
bool IsZero(double value) {
    return abs(value) < EPSILON;
}

svg::Polyline MapRenderer::DrawRoute(const vector<svg::Point>& points, int i) const {
    svg::Color line_color = settings_.color_palette[i % settings_.color_palette.size()];
    svg::Polyline line;
    line.SetStrokeColor(line_color).SetFillColor("none"s).SetStrokeWidth(settings_.line_width).
        SetStrokeLineCap(svg::StrokeLineCap::ROUND).
            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    for (const auto& point : points) {
        line.AddPoint(point);
    }
    return line;
}
    
pair<svg::Text, svg::Text> MapRenderer::DrawName(svg::Point point, string_view route_name, int i) const {
    svg::Color line_color = settings_.color_palette[i % settings_.color_palette.size()];
    svg::Text name;
    name.SetFillColor(line_color).SetPosition(point).SetOffset(settings_.bus_label_offset).
        SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s).
        SetData(static_cast<string>(route_name));
    
    svg::Text subs = name;
    subs.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).
        SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).
        SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
 
    return {subs, name};
}
    
svg::Circle MapRenderer::DrawStop(svg::Point point) const {
    svg::Circle circle;
    circle.SetCenter(point).SetRadius(settings_.stop_radius).SetFillColor("white"s);
    return circle;
}
    
void MapRenderer::DrawRoutes(svg::Document& doc, const map<string_view, TrC::Bus*>& routes,
                           const SphereProjector& proj) const {
    int i = 0;
    for (const auto [key, value] : routes) {
        if (!value->route.size()) {
            continue;
        }
        
        vector<svg::Point> points;
        for (const auto stop : value->route) {
            points.push_back(proj(stop->coord));
        }
        if (!value->is_ring) {
            points.insert(points.end(), points.rbegin()+1, points.rend());
        }
        doc.Add(DrawRoute(points, i));
        ++i;
    }
}

void MapRenderer::DrawRoutesNames(svg::Document& doc, const map<string_view, TrC::Bus*>& routes, 
                   const SphereProjector& proj) const {    
    int i = 0;
    for (const auto [key, value] : routes) {
        if (!value->route.size()) {
            continue;
        }
        auto text = DrawName(proj(value->route.front()->coord), key, i);
        doc.Add(text.first);
        doc.Add(text.second);
        
        if (!value->is_ring && value->route.front()->name != value->route.back()->name) {
            text = DrawName(proj(value->route.back()->coord), key, i);
            doc.Add(text.first);
            doc.Add(text.second);
        }
        ++i;
    }
}
    
void MapRenderer::DrawStops(svg::Document& doc, const map<string_view, geo::Coordinates>& stops, 
                   const SphereProjector& proj) const {
    for (const auto& [name, coord] : stops) {
        doc.Add(DrawStop(proj(coord)));
    }
}
    
void MapRenderer::DrawStopsNames(svg::Document& doc, const map<string_view, geo::Coordinates>& stops, 
                   const SphereProjector& proj) const {
    for(const auto& [name, coord] : stops) {
        auto text = DrawStopName(proj(coord), name);
        doc.Add(text.first);
        doc.Add(text.second);
    }
}
    
pair<svg::Text, svg::Text> MapRenderer::DrawStopName(svg::Point point, string_view stop_name) const {
    svg::Text name;
    name.SetPosition(point).SetOffset(settings_.stop_label_offset).SetFontSize(settings_.stop_label_font_size).
        SetFontFamily("Verdana"s).SetData(static_cast<string>(stop_name));
    svg::Text subs = name;
    subs.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).
        SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).
        SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    name.SetFillColor("black"s);
    return {subs, name};
}
    
} // namespace renderer