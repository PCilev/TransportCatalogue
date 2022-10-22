#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

class RequestHandler {
public:
    RequestHandler(TrC::TransportCatalogue& db, renderer::MapRenderer& renderer, router::Graph& graph) : 
    db_{db}, renderer_{renderer}, graph_{graph} {
    }
    
    TrC::Stop& GetStop(std::string_view name) const {
        return db_.FindStop(name);
    }
    
    TrC::Bus& GetBus(std::string_view name) const {
        return db_.FindBus(name);
    }

    std::optional<TrC::BusInfo> GetBusInfo(const std::string_view& bus_name) const;

    const std::vector<std::string> GetBusesByStop(std::string_view stop_name) const {
        std::vector<std::string_view> temp = db_.GetBusesForStop(stop_name);
        return std::vector<std::string>(temp.begin(), temp.end());
    }
    
    bool StopCount(std::string_view name) const {
        return db_.StopCount(name);
    }
    
    bool BusCount(std::string_view name) const {
        return db_.BusCount(name);
    }
    
    svg::Document RenderMap() const;
    
    void GraphInit(router::RoutingSettings settings) {
        graph_ = std::move(router::GraphInit(settings, db_));
    }
    
    router::TransportRouter MakeTransportRouter(router::RoutingSettings settings) const {
        return router::TransportRouter(settings, db_);
    }
    
    router::TransportRouter MakeTransportRouterWithGraph(router::RoutingSettings settings) const {
        return router::TransportRouter(settings, db_, graph_);
    }
    
    void Serialize(const std::string& path, router::RoutingSettings settings) {
        serialization::Serialize(path, db_, renderer_.GetSettings(), MakeTransportRouterWithGraph(settings));
    }
    
    bool Deserialize(const std::string& path, router::TransportRouter& router) {
        return serialization::Deserialize(path, db_, renderer_.GetSettings(), router);           
    }

private:
    TrC::TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;
    router::Graph& graph_;
};