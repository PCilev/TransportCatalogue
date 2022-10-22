#pragma once

#include "transport_catalogue.h"
#include "transport_router.h"

#include "transport_catalogue.pb.h"

namespace serialization {
    
using RouteData = graph::Router<router::RouteWeight>::RouteInternalData;
using RoutesData = std::vector<std::optional<RouteData>>;

serialize::Stop SaveStop(const TrC::Stop& stop, int id);
TrC::Stop LoadStop(const serialize::Stop& pb_stop);
    
serialize::Bus SaveBus(const TrC::Bus& bus, int id, const TrC::TransportCatalogue& catalogue);
TrC::Bus LoadBus(const serialize::Bus& pb_bus, const TrC::TransportCatalogue& catalogue);
    
serialize::Rgb SaveRgb(const svg::Rgb& rgb);
svg::Rgb LoadRgb(const serialize::Rgb& pb_rgb);
    
serialize::Rgba SaveRgba(const svg::Rgba& rgba);
svg::Rgba LoadRgba(const serialize::Rgba& pb_rgba);
    
serialize::Color SaveColor(const svg::Color& color);
svg::Color LoadColor(const serialize::Color& pb_color);
    
serialize::Point SavePoint(const svg::Point& point);
svg::Point LoadPoint(const serialize::Point& pb_point);
    
serialize::RenderSettings SaveRenderSettings(const renderer::RenderSettings& settings);
renderer::RenderSettings LoadRenderSettings(const serialize::RenderSettings& pb_settings);
    
serialize::RouteWeight SaveRouteWeight(const router::RouteWeight& weight, 
                                        const TrC::TransportCatalogue& catalogue);
router::RouteWeight LoadRouteWeight(const serialize::RouteWeight& pb_weight, 
                                        const TrC::TransportCatalogue& catalogue);
    
serialize::Edge SaveEdge(const graph::Edge<router::RouteWeight>& edge, 
                                            const TrC::TransportCatalogue& catalogue);
graph::Edge<router::RouteWeight> LoadEdge(const serialize::Edge& pb_edge, 
                                            const TrC::TransportCatalogue& catalogue);
    
serialize::EdgeId SaveEdgeId(graph::EdgeId edge_id);
graph::EdgeId LoadEdgeId(serialize::EdgeId pb_edge_id);
    
serialize::IncidenceList SaveIncidenceList(const std::vector<graph::EdgeId>& incidence_list);
std::vector<graph::EdgeId> LoadIncidenceList(const serialize::IncidenceList& pb_incidence_list);
    
serialize::Graph SaveGraph(const router::Graph& graph, const TrC::TransportCatalogue& catalogue);
router::Graph LoadGraph(const serialize::Graph& pb_graph, const TrC::TransportCatalogue& catalogue);
    
serialize::RouteInternalData SaveRouteInternalData(const RouteData& data, 
                                            const TrC::TransportCatalogue& catalogue);
RouteData LoadRouteInternalData(const serialize::RouteInternalData& pb_data, 
                                            const TrC::TransportCatalogue& catalogue);
    
serialize::RoutesInternalData SaveRoutesInternalData(const RoutesData& data, 
                                            const TrC::TransportCatalogue& catalogue);
RoutesData LoadRoutesInternalData(const serialize::RoutesInternalData& pb_data, 
                                            const TrC::TransportCatalogue& catalogue);
    
serialize::Router SaveRouter(const graph::Router<router::RouteWeight>& router, 
                                            const TrC::TransportCatalogue& catalogue);
    
void LoadRouter(const serialize::Router& pb_router, graph::Router<router::RouteWeight>& router, 
                                            const TrC::TransportCatalogue& catalogue);
    
serialize::RoutingSettings SaveRoutingSettings(const router::RoutingSettings& settings);
router::RoutingSettings LoadRoutingSettings(const serialize::RoutingSettings& pb_settings);
    
serialize::TransportRouter SaveTransportRouter(const router::TransportRouter& router, 
                                            const TrC::TransportCatalogue& catalogue);
void LoadTransportRouter(const serialize::TransportRouter& pb_router, router::TransportRouter& router, 
                                            const TrC::TransportCatalogue& catalogue);
    
serialize::TransportCatalogue SaveTransportCatalogue(const TrC::TransportCatalogue& catalogue,
                    const renderer::RenderSettings& settings, const router::TransportRouter& router);
void LoadTransportCatalogue(const serialize::TransportCatalogue& pb_catalogue, 
                                              TrC::TransportCatalogue& catalogue);
    
void Serialize(const std::string& path, const TrC::TransportCatalogue& catalogue,
              const renderer::RenderSettings& settings, const router::TransportRouter& router);
bool Deserialize(const std::string& path, TrC::TransportCatalogue& catalogue,
                renderer::RenderSettings& settings, router::TransportRouter& router);
    
} // namespace serialization