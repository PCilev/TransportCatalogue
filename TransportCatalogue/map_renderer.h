#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>

namespace renderer {

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct RenderSettings {
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    double underlayer_width;
    
    int bus_label_font_size;
    int stop_label_font_size;
    
    svg::Point bus_label_offset;
    svg::Point stop_label_offset;
    
    svg::Color underlayer_color;
    std::vector<svg::Color> color_palette;
};

class MapRenderer {
public:
    MapRenderer() = default;
    MapRenderer(const RenderSettings& settings);
    
    void SetSettings(const RenderSettings& settings) {
        settings_ = settings;
    }
    
    const RenderSettings& GetSettings() const {
        return settings_;
    }
    
    RenderSettings& GetSettings() {
        return settings_;
    }
    
    svg::Polyline DrawRoute(const std::vector<svg::Point>& points, int i) const;
    std::pair<svg::Text, svg::Text> DrawName(svg::Point point, std::string_view route_name, int i) const;
    svg::Circle DrawStop(svg::Point point) const;
    std::pair<svg::Text, svg::Text> DrawStopName(svg::Point point, std::string_view stop_name) const;
    
    void DrawRoutes(svg::Document& doc, const std::map<std::string_view, TrC::Bus*>& routes, 
                   const SphereProjector& proj) const;
    void DrawRoutesNames(svg::Document& doc, const std::map<std::string_view, TrC::Bus*>& routes, 
                   const SphereProjector& proj) const;
    void DrawStops(svg::Document& doc, const std::map<std::string_view, geo::Coordinates>& stops, 
                   const SphereProjector& proj) const;
    void DrawStopsNames(svg::Document& doc, const std::map<std::string_view, geo::Coordinates>& stops, 
                   const SphereProjector& proj) const;
    
private:
    RenderSettings settings_;
};
    
} // namespace renderer