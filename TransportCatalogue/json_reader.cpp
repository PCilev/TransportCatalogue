#include "json_reader.h"

using namespace std;

svg::Color ToColor(const json::Node& node) {
    if (node.IsArray()) {
        json::Array arr = node.AsArray();
        if (arr.size() == 3) {
            return svg::Rgb(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
        }
        else {
            return svg::Rgba(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
        }
    }
    return node.AsString();
}

TrC::Stop JsonReader::MakeStop(const json::Dict& stop) const {
    return {stop.at("name"s).AsString(), 
            {stop.at("latitude"s).AsDouble(), stop.at("longitude"s).AsDouble()}};
}

TrC::Bus JsonReader::MakeBus(const RequestHandler& handler, const json::Dict& bus) const {
    vector<TrC::Stop*> stops;
    for (const auto& name : bus.at("stops"s).AsArray()) {
        stops.push_back(&handler.GetStop(name.AsString()));
    }

    return {bus.at("name"s).AsString(), stops, bus.at("is_roundtrip"s).AsBool()};
}

pair<TrC::Stop*, vector<TrC::detail::DistanceToStop>> JsonReader::
MakeDistances(const RequestHandler& handler, const json::Dict& stop) const {
    vector<TrC::detail::DistanceToStop> distances;
    for (const auto& [key, value] : stop.at("road_distances"s).AsDict()) {
        distances.push_back({static_cast<unsigned>(value.AsInt()), key});
    }
    return {&handler.GetStop(stop.at("name"s).AsString()), distances};
}

void JsonReader::FillingCatalogue(TrC::TransportCatalogue& catalogue, const RequestHandler& handler) const {
    for(const auto& value : querys_.GetRoot().AsDict().at("base_requests"s).AsArray()) {
        if (value.AsDict().at("type"s).AsString() == "Stop"s) {
            catalogue.AddStop(MakeStop(value.AsDict()));
        }
    }
    
    for(const auto& value : querys_.GetRoot().AsDict().at("base_requests"s).AsArray()) {
        if (value.AsDict().at("type"s).AsString() == "Bus"s) {
            catalogue.AddBus(MakeBus(handler, value.AsDict()));
        }
    }
    
    for(const auto& value : querys_.GetRoot().AsDict().at("base_requests"s).AsArray()) {
        if (value.AsDict().at("type"s).AsString() == "Stop"s) {
            catalogue.AddDistances(MakeDistances(handler, value.AsDict()));
        }
    }
}

void MakeStopReport(const json::Dict& request, const RequestHandler& handler, json::Dict& map) {
    if (!handler.StopCount(request.at("name"s).AsString())) {
        map["error_message"s] = "not found"s;
        return;
    }
    const vector<string> temp = handler.GetBusesByStop(request.at("name"s).AsString());
    json::Array arr(temp.begin(), temp.end());
    map["buses"s] = move(arr);
}

void MakeBusReport(const json::Dict& request, const RequestHandler& handler, json::Dict& map) {
    std::optional<TrC::BusInfo> info = handler.GetBusInfo(request.at("name"s).AsString());
    if (!info) {
        map["error_message"s] = "not found"s;
        return;
    }
    map["curvature"s] = info->real_length / info->length;
    map["route_length"s] = info->real_length;
    map["stop_count"s] = info->stops;
    map["unique_stop_count"s] = info->unique_stops;
}

void MakeRouteReport(const json::Dict& request, router::TransportRouter& router, json::Dict& map) {
    const auto& from = request.at("from"s).AsString();
    const auto& to = request.at("to"s).AsString();

    auto route = router.BuildRoute(from, to);
    if (!route.has_value()) {
        map["error_message"s] = "not found";
        return;
    }

    double route_time = 0;
    int wait_time = router.GetSettings().wait_time;
    json::Array items;
    for (const auto& edge : route.value()) {
        route_time += edge.route_time;
        json::Dict wait_elem = json::Builder{}.StartDict().Key("type"s).Value("Wait"s).
        Key("stop_name"s).Value(std::string(edge.stop_from)).Key("time"s).Value(wait_time).
        EndDict().Build().AsDict();
        
        json::Dict ride_elem = json::Builder{}.StartDict().Key("type"s).Value("Bus"s).
        Key("bus"s).Value(std::string(edge.bus_name)).Key("span_count"s).Value(edge.span_count).
        Key("time"s).Value(edge.route_time - wait_time).EndDict().Build().AsDict();
        
        items.push_back(wait_elem);
        items.push_back(ride_elem);
    }
    map["total_time"s] = route_time;
    map["items"s] = items;
}

json::Document JsonReader::MakeReport(const RequestHandler& handler, router::TransportRouter& router) const {
    //router::TransportRouter router = handler.MakeTransportRouterWithGraph(ReadRoutingSettings());
    json::Array data;
    data.reserve(querys_.GetRoot().AsDict().at("stat_requests"s).AsArray().size());
    for (const auto& value : querys_.GetRoot().AsDict().at("stat_requests"s).AsArray()) {
        json::Dict map;
        map["request_id"s] = value.AsDict().at("id"s).AsInt();
        if (value.AsDict().at("type"s).AsString() == "Stop"s) {
            MakeStopReport(value.AsDict(), handler, map);
        }
        else if (value.AsDict().at("type"s).AsString() == "Bus"s) {
            MakeBusReport(value.AsDict(), handler, map);
        } 
        else if (value.AsDict().at("type"s).AsString() == "Route"s) {
            MakeRouteReport(value.AsDict(), router, map);
            }
        else {
            svg::Document doc = handler.RenderMap();
            ostringstream ost;
            doc.Render(ost);
            map["map"s] = ost.str();
        }
        data.emplace_back(map);
    }
    
    return json::Document(json::Builder{}.Value(data).Build());
}

renderer::RenderSettings JsonReader::ReadRenderSettings() const {
    renderer::RenderSettings settings;
    json::Dict map = querys_.GetRoot().AsDict().at("render_settings"s).AsDict();
    settings.width = map.at("width"s).AsDouble();
    settings.height = map.at("height"s).AsDouble();
    settings.padding = map.at("padding"s).AsDouble();
    settings.line_width = map.at("line_width"s).AsDouble();
    settings.stop_radius = map.at("stop_radius"s).AsDouble();
    settings.underlayer_width = map.at("underlayer_width"s).AsDouble();
    
    settings.bus_label_font_size = map.at("bus_label_font_size"s).AsInt();
    settings.stop_label_font_size = map.at("stop_label_font_size"s).AsInt();
    
    json::Array dxdy = map.at("bus_label_offset"s).AsArray();
    settings.bus_label_offset = {dxdy[0].AsDouble(), dxdy[1].AsDouble()};
    dxdy = map.at("stop_label_offset"s).AsArray();
    settings.stop_label_offset = {dxdy[0].AsDouble(), dxdy[1].AsDouble()};
    
    settings.underlayer_color = ToColor(map.at("underlayer_color"s));
    for (const auto& color : map.at("color_palette"s).AsArray()) {
        settings.color_palette.push_back(ToColor(color));
    }
    
    return settings;
}

router::RoutingSettings JsonReader::ReadRoutingSettings() const {
    json::Dict map = querys_.GetRoot().AsDict().at("routing_settings"s).AsDict();
    return {map.at("bus_wait_time"s).AsInt(), map.at("bus_velocity"s).AsDouble()};
}

string JsonReader::ReadSerializationSettings() const {
    json::Dict map = querys_.GetRoot().AsDict().at("serialization_settings"s).AsDict();
    return map.at("file"s).AsString();
}