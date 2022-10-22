#include "serialization.h"
#include <fstream>

namespace serialization {

using namespace std;

serialize::Stop SaveStop(const TrC::Stop& stop, int id) {
    serialize::Stop pb_stop;
    pb_stop.set_name(stop.name);
    
    serialize::Coordinates c;
    c.set_lat(stop.coord.lat);
    c.set_lng(stop.coord.lng);
    *pb_stop.mutable_coord() = c;
    
    pb_stop.set_id(id);
    return pb_stop;
}

TrC::Stop LoadStop(const serialize::Stop& pb_stop) {
    return {pb_stop.name(), {pb_stop.coord().lat(), pb_stop.coord().lng()}};
}
    
serialize::Bus SaveBus(const TrC::Bus& bus, int id, const TrC::TransportCatalogue& catalogue) {
    serialize::Bus pb_bus;
    pb_bus.set_name(bus.name);
    pb_bus.set_is_ring(bus.is_ring);
    
    for (const auto stop : bus.route) {
        pb_bus.add_route(distance(catalogue.GetStops().begin(), catalogue.GetStops().find(stop->name)));
    }
    
    pb_bus.set_id(id);
    return pb_bus;
}
    
TrC::Bus LoadBus(const serialize::Bus& pb_bus, const TrC::TransportCatalogue& catalogue) {
    vector<TrC::Stop*> route;
    for (size_t i = 0; i < pb_bus.route_size(); ++i) {
        route.push_back(next(catalogue.GetStops().begin(), pb_bus.route(i))->second);
    }
    
    return {pb_bus.name(), route, pb_bus.is_ring()};
}
    
serialize::Rgb SaveRgb(const svg::Rgb& rgb) {
    serialize::Rgb pb_rgb;
    
    pb_rgb.set_red(rgb.red);
    pb_rgb.set_green(rgb.green);
    pb_rgb.set_blue(rgb.blue);
    
    return pb_rgb;
}
    
svg::Rgb LoadRgb(const serialize::Rgb& pb_rgb) {
    return svg::Rgb(pb_rgb.red(), pb_rgb.green(), pb_rgb.blue());
}
    
serialize::Rgba SaveRgba(const svg::Rgba& rgba) {
    serialize::Rgba pb_rgba;
    
    pb_rgba.set_red(rgba.red);
    pb_rgba.set_green(rgba.green);
    pb_rgba.set_blue(rgba.blue);
    pb_rgba.set_opacity(rgba.opacity);
    
    return pb_rgba;
}
    
svg::Rgba LoadRgba(const serialize::Rgba& pb_rgba) {
    return svg::Rgba(pb_rgba.red(), pb_rgba.green(), pb_rgba.blue(), pb_rgba.opacity());
}
    
serialize::Color SaveColor(const svg::Color& color) {
    serialize::Color pb_color;
    
    if (std::holds_alternative<string>(color)) {
        pb_color.set_name(std::get<string>(color));
    }
    else if (std::holds_alternative<svg::Rgb>(color)) {
        *pb_color.mutable_rgb() = SaveRgb(std::get<svg::Rgb>(color));
    }
    else if (std::holds_alternative<svg::Rgba>(color)){
        *pb_color.mutable_rgba() = SaveRgba(std::get<svg::Rgba>(color));
    }
    
    return pb_color;
}
    
svg::Color LoadColor(const serialize::Color& pb_color)  {
    svg::Color color;
        
    if (!pb_color.name().empty()) {
        color = move(pb_color.name());
    }
    else if (pb_color.has_rgb()) {
        color = move(LoadRgb(pb_color.rgb()));
    }
    else if (pb_color.has_rgba()) {
        color = move(LoadRgba(pb_color.rgba()));
    }
    
    return color;
}
    
serialize::Point SavePoint(const svg::Point& point) {
    serialize::Point pb_point;
    
    pb_point.set_x(point.x);
    pb_point.set_y(point.y);
    
    return pb_point;
}
    
svg::Point LoadPoint(const serialize::Point& pb_point) {
    return svg::Point(pb_point.x(), pb_point.y());
}
    
serialize::RenderSettings SaveRenderSettings(const renderer::RenderSettings& settings) {
    serialize::RenderSettings pb_settings;
    
    pb_settings.set_width(settings.width);
    pb_settings.set_height(settings.height);
    pb_settings.set_padding(settings.padding);
    pb_settings.set_line_width(settings.line_width);
    pb_settings.set_stop_radius(settings.stop_radius);
    pb_settings.set_underlayer_width(settings.underlayer_width);
    pb_settings.set_bus_label_font_size(settings.bus_label_font_size);
    pb_settings.set_stop_label_font_size(settings.stop_label_font_size);
    *pb_settings.mutable_bus_label_offset() = SavePoint(settings.bus_label_offset);
    *pb_settings.mutable_stop_label_offset() = SavePoint(settings.stop_label_offset);
    *pb_settings.mutable_underlayer_color() = SaveColor(settings.underlayer_color);
    
    for (const auto& color : settings.color_palette) {
        *pb_settings.add_color_palette() = SaveColor(color);
    }
    
    return pb_settings;
}
    
renderer::RenderSettings LoadRenderSettings(const serialize::RenderSettings& pb_settings) {
    renderer::RenderSettings settings;
    
    settings.width = pb_settings.width();
    settings.height = pb_settings.height();
    settings.padding = pb_settings.padding();
    settings.line_width = pb_settings.line_width();
    settings.stop_radius = pb_settings.stop_radius();
    settings.underlayer_width = pb_settings.underlayer_width();
    settings.bus_label_font_size = pb_settings.bus_label_font_size();
    settings.stop_label_font_size = pb_settings.stop_label_font_size();
    settings.bus_label_offset = LoadPoint(pb_settings.bus_label_offset());
    settings.stop_label_offset = LoadPoint(pb_settings.stop_label_offset());
    settings.underlayer_color = LoadColor(pb_settings.underlayer_color());
    
    for (size_t i = 0; i < pb_settings.color_palette_size(); ++i) {
        settings.color_palette.push_back(LoadColor(pb_settings.color_palette(i)));
    }
    
    return settings;
}
    
serialize::RouteWeight SaveRouteWeight(const router::RouteWeight& weight, 
                                        const TrC::TransportCatalogue& catalogue) {
    serialize::RouteWeight pb_weight;
    auto it = std::find_if(catalogue.BusesBegin(), catalogue.BusesEnd(), [&weight](const TrC::Bus& bus){
        return bus.name == weight.bus_name;
    });
    
    pb_weight.set_bus_id(std::distance(catalogue.BusesBegin(), it));
    pb_weight.set_route_time(weight.route_time);
    pb_weight.set_span_count(weight.span_count);
    
    return pb_weight;
}
    
router::RouteWeight LoadRouteWeight(const serialize::RouteWeight& pb_weight, 
                                        const TrC::TransportCatalogue& catalogue) {
    return {(catalogue.BusesBegin() + pb_weight.bus_id())->name, 
            pb_weight.route_time(), static_cast<int>(pb_weight.span_count())};
}
    
serialize::Edge SaveEdge(const graph::Edge<router::RouteWeight>& edge, 
                                        const TrC::TransportCatalogue& catalogue) {
    serialize::Edge pb_edge;
    
    pb_edge.set_stop_from_id(edge.from);
    pb_edge.set_stop_to_id(edge.to);
    *pb_edge.mutable_weight() = SaveRouteWeight(edge.weight, catalogue);
    
    return pb_edge;
}
    
graph::Edge<router::RouteWeight> LoadEdge(const serialize::Edge& pb_edge, 
                                        const TrC::TransportCatalogue& catalogue) {
    return {pb_edge.stop_from_id(), pb_edge.stop_to_id(), 
            LoadRouteWeight(pb_edge.weight(), catalogue)};
}
    
serialize::EdgeId SaveEdgeId(graph::EdgeId edge_id) {
    serialize::EdgeId pb_edge_id;
    
    pb_edge_id.set_edge_id(edge_id);
    return pb_edge_id;
}
    
graph::EdgeId LoadEdgeId(serialize::EdgeId pb_edge_id) {
    return pb_edge_id.edge_id();
}
    
serialize::IncidenceList SaveIncidenceList(const vector<graph::EdgeId>& incidence_list) {
    serialize::IncidenceList pb_incidence_list;
    
    for (const auto edge_id : incidence_list) {
        *pb_incidence_list.add_edge_id() = SaveEdgeId(edge_id);
    }
    
    return pb_incidence_list;
}
    
vector<graph::EdgeId> LoadIncidenceList(const serialize::IncidenceList& pb_incidence_list) {
    vector<graph::EdgeId> incidence_list;
    
    for (size_t i = 0; i < pb_incidence_list.edge_id_size(); ++i) {
        incidence_list.push_back(LoadEdgeId(pb_incidence_list.edge_id(i)));
    }
    
    return incidence_list;
}
    
serialize::Graph SaveGraph(const router::Graph& graph, 
                                            const TrC::TransportCatalogue& catalogue) {
    serialize::Graph pb_graph;
    
    for (const auto& edge : graph.GetEdges()) {
        *pb_graph.add_edges() = SaveEdge(edge, catalogue);
    }
    
    for (const auto& v_icidense_list : graph.GetIncidenceLists()) {
        *pb_graph.add_incidence_lists() = move(SaveIncidenceList(v_icidense_list));
    }
    
    return pb_graph;
}
    
router::Graph LoadGraph(const serialize::Graph& pb_graph, 
                        const TrC::TransportCatalogue& catalogue) {
    router::Graph graph;
    
    for (size_t i = 0; i < pb_graph.edges_size(); ++i) {
        graph.GetEdges().push_back(LoadEdge(pb_graph.edges(i), catalogue));
    }
    
    for (size_t i = 0; i < pb_graph.incidence_lists_size(); ++i) {
        graph.GetIncidenceLists().push_back(move(LoadIncidenceList(pb_graph.incidence_lists(i))));
    }
    
    return graph;
}
    
serialize::RouteInternalData SaveRouteInternalData(const RouteData& data, 
                                                const TrC::TransportCatalogue& catalogue) {
    serialize::RouteInternalData pb_data;
    
    *pb_data.mutable_weight() = SaveRouteWeight(data.weight, catalogue);
    if (data.prev_edge) {
        *pb_data.mutable_prev_edge() = SaveEdgeId(data.prev_edge.value());
    }
    
    return pb_data;
}
    
RouteData LoadRouteInternalData(const serialize::RouteInternalData& pb_data, 
                                                const TrC::TransportCatalogue& catalogue) {
    graph::EdgeId edge_id;
    if (pb_data.has_prev_edge()) {
        edge_id = LoadEdgeId(pb_data.prev_edge());
        return {LoadRouteWeight(pb_data.weight(), catalogue), edge_id};
    }
    else {
        return {LoadRouteWeight(pb_data.weight(), catalogue), nullopt};
    }
}
    
serialize::RoutesInternalData SaveRoutesInternalData(const RoutesData& data, 
                                                const TrC::TransportCatalogue& catalogue) {
    serialize::RoutesInternalData pb_data;
    
    for (const auto& route_iternal_data : data) {
        serialize::OptRouteInternalData opt_route_data;
        if (route_iternal_data) {
            *opt_route_data.mutable_opt_data() = SaveRouteInternalData(route_iternal_data.value(), catalogue);
        }
        *pb_data.add_data() = opt_route_data;
    }
    
    return pb_data;
}
    
RoutesData LoadRoutesInternalData(const serialize::RoutesInternalData& pb_data,
                                                const TrC::TransportCatalogue& catalogue) {
    RoutesData data;
    
    for (size_t i = 0; i < pb_data.data_size(); ++i) {
        if (pb_data.data(i).has_opt_data()) {
            data.emplace_back(LoadRouteInternalData(pb_data.data(i).opt_data(), catalogue));
        }
        else {
            data.emplace_back(nullopt);
        }
    }
    
    return data;
}
    
serialize::Router SaveRouter(const graph::Router<router::RouteWeight>& router, 
                                                const TrC::TransportCatalogue& catalogue) {
    serialize::Router pb_router;
    
    for (const auto& v_internal_data : router.GetRoutesInternalData()) {
        *pb_router.add_routes_internal_data() = move(SaveRoutesInternalData(v_internal_data, catalogue));
    }
    
    return pb_router;
}
    
void LoadRouter(const serialize::Router& pb_router, graph::Router<router::RouteWeight>& router, 
               const TrC::TransportCatalogue& catalogue) {
    for (size_t i = 0; i < pb_router.routes_internal_data_size(); ++i) {
        router.GetRoutesInternalData().push_back(move(LoadRoutesInternalData(pb_router.routes_internal_data(i), catalogue)));
    }
}
    
serialize::RoutingSettings SaveRoutingSettings(const router::RoutingSettings& settings) {
    serialize::RoutingSettings pb_settings;
    
    pb_settings.set_wait_time(settings.wait_time);
    pb_settings.set_velocity(settings.velocity);
    
    return pb_settings;
}
    
router::RoutingSettings LoadRoutingSettings(const serialize::RoutingSettings& pb_settings) {
    return {static_cast<int>(pb_settings.wait_time()), pb_settings.velocity()};
}
    
serialize::TransportRouter SaveTransportRouter(const router::TransportRouter& router, 
                                                const TrC::TransportCatalogue& catalogue) {
    serialize::TransportRouter pb_router;
    
    *pb_router.mutable_settings() = SaveRoutingSettings(router.GetRoutingSettings());
    *pb_router.mutable_graph() = move(SaveGraph(router.GetGraph(), catalogue));
    *pb_router.mutable_router() = move(SaveRouter(router.GetRouter(), catalogue));
    
    return pb_router;
}
    
void LoadTransportRouter(const serialize::TransportRouter& pb_router, router::TransportRouter& router, 
                                            const TrC::TransportCatalogue& catalogue) {
    router.SetRoutingSettings(LoadRoutingSettings(pb_router.settings()));
    router.GetGraph() = move(LoadGraph(pb_router.graph(), catalogue));
    LoadRouter(pb_router.router(), router.GetRouter(), catalogue);
}
    
serialize::TransportCatalogue SaveTransportCatalogue(const TrC::TransportCatalogue& catalogue,
              const renderer::RenderSettings& settings, const router::TransportRouter& router) {
    serialize::TransportCatalogue pb_catalogue;
    int i = 0;
    for (auto it = catalogue.StopsBegin(); it != catalogue.StopsEnd(); ++it) {
        *pb_catalogue.add_stops() = SaveStop(*it, i);
        ++i;
    }
    
    i = 0;
    for (auto it = catalogue.BusesBegin(); it != catalogue.BusesEnd(); ++it) {
        *pb_catalogue.add_buses() = SaveBus(*it, i, catalogue);
        ++i;
    }
    
    for (const auto& [stop_pair, distance] : catalogue.GetDistances()) {
        serialize::Distance pb_distance;
        pb_distance.set_from_id(std::distance(catalogue.GetStops().begin(), 
                                         catalogue.GetStops().find(stop_pair.first->name)));
        pb_distance.set_to_id(std::distance(catalogue.GetStops().begin(), 
                                         catalogue.GetStops().find(stop_pair.second->name)));
        pb_distance.set_distance(distance);
        
        *pb_catalogue.add_distances() = pb_distance;
    }
    
    *pb_catalogue.mutable_render_settings() = move(SaveRenderSettings(settings));
    *pb_catalogue.mutable_router() = move(SaveTransportRouter(router, catalogue));
    
    return pb_catalogue;
}
    
void LoadTransportCatalogue(const serialize::TransportCatalogue& pb_catalogue, 
                                               TrC::TransportCatalogue& catalogue) {
    
    for (size_t i = 0; i < pb_catalogue.stops_size(); ++i) {
        catalogue.AddStop(LoadStop(pb_catalogue.stops(i)));
    }
    
    for (size_t i = 0; i < pb_catalogue.buses_size(); ++i) {
        catalogue.AddBus(LoadBus(pb_catalogue.buses(i), catalogue));
    }
    
    for (size_t i = 0; i < pb_catalogue.distances_size(); ++i) {
        serialize::Distance pb_distance = pb_catalogue.distances(i);
        TrC::Stop* from_ptr = next(catalogue.GetStops().begin(), pb_distance.from_id())->second;
        TrC::Stop* to_ptr = next(catalogue.GetStops().begin(), pb_distance.to_id())->second;
        
        catalogue.AddDistances(from_ptr, to_ptr, pb_distance.distance());
    }
}
    
void Serialize(const string& path, const TrC::TransportCatalogue& catalogue, 
               const renderer::RenderSettings& settings, const router::TransportRouter& router) {
    ofstream out(path, ios::binary);
    serialize::TransportCatalogue pb_catalogue = move(SaveTransportCatalogue(catalogue, settings, router));
    pb_catalogue.SerializeToOstream(&out);
}
    
bool Deserialize(const string& path, TrC::TransportCatalogue& catalogue, renderer::RenderSettings& settings, 
                                                    router::TransportRouter& router) {
    ifstream in_file(path, ios::binary);
    serialize::TransportCatalogue pb_catalogue;
    if (!pb_catalogue.ParseFromIstream(&in_file)) {
        return false;
    }
    
    LoadTransportCatalogue(pb_catalogue, catalogue);
    settings = LoadRenderSettings(pb_catalogue.render_settings());
    LoadTransportRouter(pb_catalogue.router(), router, catalogue);
    
    return true;
}
    
} // namespace serialization 
