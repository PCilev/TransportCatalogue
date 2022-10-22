#include "request_handler.h"

#include <map>

using namespace std;

optional<TrC::BusInfo> RequestHandler::GetBusInfo(const string_view& bus_name) const {
    try {
         return db_.GetBusInfo(bus_name);
    }
    catch (const std::out_of_range& e) {
        return nullopt;
    }
}

svg::Document RequestHandler::RenderMap() const {
    svg::Document doc;
    map<string_view, TrC::Bus*> routes;
    vector<geo::Coordinates> coord;
    
    for (const auto& [key, value] : db_.GetBuses()) {
        routes[key] = value;
        for (const auto stop : value->route) {
            coord.push_back(stop->coord);
        }
    }
    
    renderer::SphereProjector proj(coord.begin(), coord.end(), renderer_.GetSettings().width,
        renderer_.GetSettings().height, renderer_.GetSettings().padding);
    
    renderer_.DrawRoutes(doc, routes, proj);
    renderer_.DrawRoutesNames(doc, routes, proj);
    
    map<string_view, geo::Coordinates> stops;
    for (const auto [key, value] : routes) {
        for (const auto stop : value->route) {
            stops[stop->name] = stop->coord;
        }
    }
    
    renderer_.DrawStops(doc, stops, proj);
    renderer_.DrawStopsNames(doc, stops, proj);
    
    return doc;
}
