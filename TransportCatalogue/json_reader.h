#pragma once

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_builder.h"

svg::Color ToColor(const json::Node& node);

class JsonReader {
public:
    JsonReader() = default;
    explicit JsonReader(const json::Document& querys) : querys_{ querys } {
    }
    explicit JsonReader(std::istream& ist) : querys_{ json::Load(ist) } {
    }
    JsonReader(const JsonReader&) = delete;
    
    JsonReader& operator=(const JsonReader&) = delete;
    
    void SetQuerys(const json::Document& querys) {
        querys_ = querys;
    }
    
    const json::Document& GetQuerys() const {
        return querys_;
    }
    
    void FillingCatalogue(TrC::TransportCatalogue& catalogue, const RequestHandler& handler) const;
    
    json::Document MakeReport(const RequestHandler& handler, router::TransportRouter& router) const;
    
    renderer::RenderSettings ReadRenderSettings() const;
    
    router::RoutingSettings ReadRoutingSettings() const;
    
    std::string ReadSerializationSettings() const;
    
private:
    TrC::Stop MakeStop(const json::Dict& stop) const;
    TrC::Bus MakeBus(const RequestHandler& handler, const json::Dict& bus) const;
    std::pair<TrC::Stop*,std::vector<TrC::detail::DistanceToStop>> 
        MakeDistances(const RequestHandler& handler, const json::Dict& stop) const;
    
    json::Document querys_;
};

void MakeStopReport(const json::Dict& request, const RequestHandler& handler, json::Dict& map);

void MakeBusReport(const json::Dict& request, const RequestHandler& handler, json::Dict& map);

void MakeRouteReport(const json::Dict& request, router::TransportRouter& router, json::Dict& map);