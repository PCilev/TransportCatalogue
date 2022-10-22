#include "transport_router.h"

namespace router {
    
using namespace std;
    
bool operator<(const RouteWeight& lvl, const RouteWeight& rvl) {
    return lvl.route_time < rvl.route_time;
}  
    
bool operator>(const RouteWeight& lvl, const RouteWeight& rvl) {
    return lvl.route_time > rvl.route_time;
}
    
RouteWeight operator+(const RouteWeight& lvl, const RouteWeight& rvl) {
    return {string_view(), lvl.route_time + rvl.route_time, 0};
}
    
graph::Edge<RouteWeight> MakeEdge(TrC::Bus* bus, const TrC::TransportCatalogue& catalogue,
                                                size_t stop_from, size_t stop_to) {
    graph::Edge<RouteWeight> edge;
    edge.weight.bus_name = bus->name;
    edge.from = distance(catalogue.GetStops().begin(), 
                            catalogue.GetStops().find(bus->route.at(stop_from)->name));
    edge.to = distance(catalogue.GetStops().begin(), 
                            catalogue.GetStops().find(bus->route.at(stop_to)->name));
    edge.weight.span_count = stop_to - stop_from;
    
    return edge;
}

double Distance(TrC::Bus* bus, const TrC::TransportCatalogue& catalogue,
                        size_t stop_from, size_t stop_to) {
    return catalogue.StopsDistance({bus->route.at(stop_from), bus->route.at(stop_to)});
}

    
Graph GraphInit(RoutingSettings settings, const TrC::TransportCatalogue& catalogue) {
    Graph graph(catalogue.GetStops().size());  
    
    for (const auto& [bus_name, bus] : catalogue.GetBuses()) {
        for(size_t i = 0; i < bus->route.size() - 1; ++i) {
            double route_time = settings.wait_time;
            double return_route_time = route_time;
            for(size_t j = i + 1; j < bus->route.size(); ++j) {
                graph::Edge<RouteWeight> edge = MakeEdge(bus, catalogue, i, j);
                route_time += Distance(bus, catalogue, j - 1, j) / (settings.velocity * factor);
                edge.weight.route_time = route_time;
                graph.AddEdge(edge);
            
                if (!bus->is_ring) {
                    graph::Edge<RouteWeight> edge = MakeEdge(bus, catalogue, bus->route.size() - 1 - i, 
                                                                            bus->route.size() - 1 - j);
                    return_route_time += Distance(bus, catalogue, bus->route.size() - j, 
                                             bus->route.size() - 1 - j) / (settings.velocity * factor);
                    edge.weight.route_time = return_route_time;
                    graph.AddEdge(edge);
                }
            }
        }
    }
    return graph;
}

optional<vector<RouterEdge>> TransportRouter::BuildRoute(const string& from, const string& to) {
    vector<RouterEdge> result;
    if (from == to) {
        return result;
    }

    size_t from_id = distance(catalogue_.GetStops().begin(), catalogue_.GetStops().find(from));
    size_t to_id = distance(catalogue_.GetStops().begin(), catalogue_.GetStops().find(to));
    auto route = router_.BuildRoute(from_id, to_id);
    if (!route) {
        return nullopt;
    }
    
    for (auto edge_id : route->edges) {
        const auto edge = graph_.GetEdge(edge_id);
        string_view bus_name = edge.weight.bus_name;
        string_view stop_from = next(catalogue_.GetStops().begin(), edge.from)->second->name;
        string_view stop_to = next(catalogue_.GetStops().begin(), edge.to)->second->name;
        double route_time = edge.weight.route_time;
        int span_count = edge.weight.span_count;
        result.push_back({bus_name, stop_from, stop_to, route_time, span_count});
    }
    return result;
}

RoutingSettings TransportRouter::GetSettings() const {
    return settings_;
}

} // namespace router