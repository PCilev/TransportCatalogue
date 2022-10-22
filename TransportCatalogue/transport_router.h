#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

namespace router {

const double factor = 1000.0 / 60;

struct RouteWeight {
    std::string_view bus_name;
    double route_time = 0;
    int span_count = 0;
};
    
struct RoutingSettings {
    int wait_time = 0;
    double velocity = 0;
};
    
struct RouterEdge {
    std::string_view bus_name;
    std::string_view stop_from;
    std::string_view stop_to;
    double route_time = 0;
    int span_count = 0;
};
    
using Graph = graph::DirectedWeightedGraph<RouteWeight>;

bool operator<(const RouteWeight&, const RouteWeight&);
    
bool operator>(const RouteWeight&, const RouteWeight&);
    
RouteWeight operator+(const RouteWeight&, const RouteWeight&);
    
graph::Edge<RouteWeight> MakeEdge(TrC::Bus* bus, const TrC::TransportCatalogue& catalogue,
                                size_t stop_from, size_t stop_to);
    
double Distance(TrC::Bus* bus, const TrC::TransportCatalogue& catalogue,
                                size_t stop_from, size_t stop_to);
    
Graph GraphInit(RoutingSettings settings, const TrC::TransportCatalogue& catalogue);

class TransportRouter {
public:
    TransportRouter(const TrC::TransportCatalogue& catalogue) : catalogue_{catalogue}, router_(graph_) {
    }
    
    TransportRouter(RoutingSettings settings, const TrC::TransportCatalogue& catalogue) : 
    catalogue_{catalogue}, settings_{settings}, graph_{GraphInit(settings, catalogue)}, router_(graph_) {
    }
    
    TransportRouter(const TrC::TransportCatalogue& catalogue, const Graph& graph) : 
    catalogue_{catalogue}, graph_{graph}, router_(graph_) {
    }
    
    TransportRouter(RoutingSettings settings, const TrC::TransportCatalogue& catalogue, const Graph& graph) : 
    catalogue_{catalogue}, settings_{settings}, graph_{graph}, router_(graph_) {
    }
    
    TransportRouter(RoutingSettings settings, const TrC::TransportCatalogue& catalogue, const Graph& graph,
                   const graph::Router<RouteWeight>& router) : 
    catalogue_{catalogue}, settings_{settings}, graph_{graph}, router_{router} {
    }

    std::optional<std::vector<RouterEdge>> BuildRoute(const std::string& from, const std::string& to);

    RoutingSettings GetSettings() const;
    
    const Graph& GetGraph() const {
        return graph_;
    }
    
    Graph& GetGraph() {
        return graph_;
    }
    
    const graph::Router<RouteWeight>& GetRouter() const {
        return router_;
    }
    
    graph::Router<RouteWeight>& GetRouter() {
        return router_;
    }
    
    RoutingSettings GetRoutingSettings() const {
        return settings_;
    }
    
    void SetRoutingSettings(const RoutingSettings& settings) {
        settings_ = settings;
    }
    
    const TrC::TransportCatalogue& GetCatalogue() const {
        return catalogue_;
    }

private:
    const TrC::TransportCatalogue& catalogue_;
    RoutingSettings settings_;
    Graph graph_;
    graph::Router<RouteWeight> router_;
};

} // namespace router