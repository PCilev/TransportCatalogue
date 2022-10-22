#include "transport_catalogue.h"

#include <algorithm>

namespace TrC {
    
using namespace std;

void TransportCatalogue::AddStop(const Stop& stop) {
    if (!stops_names_.count(stop.name)) {
        stops_.push_back(stop);
        stops_names_[stops_.back().name] = &stops_.back();

        buses_for_stops_[&stops_.back()];
    }
}
    
void TransportCatalogue::AddStop(Stop&& stop) {
    if (!stops_names_.count(stop.name)) {
        stops_.push_back(move(stop));
        stops_names_[stops_.back().name] = &stops_.back();

        buses_for_stops_[&stops_.back()];
    }
}

void TransportCatalogue::AddDistances(Stop* from, Stop* to, unsigned distance) {
    if (!distances_.count({from, to})) {
        distances_[{from, to}] = distance;
    }
}

void TransportCatalogue::AddDistances(const std::pair<Stop*,std::vector<detail::DistanceToStop>>& distances) {
    for (const auto& dist : distances.second) {
        AddDistances(distances.first, stops_names_[dist.stop_name], dist.distance);
    }
}
    
void TransportCatalogue::AddDistances(std::pair<Stop*,std::vector<detail::DistanceToStop>>&& distances) {
    for (const auto& dist : distances.second) {
        pair<Stop*, Stop*> pr(move(distances.first), stops_names_[dist.stop_name]);
        if (!distances_.count(pr)) {
            distances_[move(pr)] = move(dist.distance);
        }
    }
}

void TransportCatalogue::AddBus(const Bus& bus) {
    if (!buses_names_.count(bus.name)) {
        buses_.push_back(bus);
        buses_names_[buses_.back().name] = &buses_.back();

        for (const auto& elem : buses_.back().route) {
            buses_for_stops_[elem].insert(buses_.back().name);
        }
    }
}
    
void TransportCatalogue::AddBus(Bus&& bus) {
    if (!buses_names_.count(bus.name)) {
        buses_.push_back(move(bus));
        buses_names_[buses_.back().name] = &buses_.back();

        for (const auto& elem : buses_.back().route) {
            buses_for_stops_[elem].insert(buses_.back().name);
        }
    }
}

Bus& TransportCatalogue::FindBus(string_view name) const {
    return *buses_names_.at(name);
}

Stop& TransportCatalogue::FindStop(string_view name) const {
    return *stops_names_.at(name);
}

BusInfo TransportCatalogue::GetBusInfo(string_view name) const {
    BusInfo info;
    Bus* bus = buses_names_.at(name);
    int num = bus->route.size();
    unordered_set<Stop*> unique_stop;
    for (size_t i = 0; i + 1 < bus->route.size(); ++i) {
        info.length += ComputeDistance(bus->route[i]->coord, bus->route[i+1]->coord);
        auto it = distances_.find({bus->route[i], bus->route[i+1]});
        if (it == distances_.end()) {
            it = distances_.find({bus->route[i+1], bus->route[i]});
        }
        info.real_length += (*it).second;

        unique_stop.insert(bus->route[i]);
    }
    unique_stop.insert(bus->route.back());

    if (!bus->is_ring) {
        info.length *= 2;
        num *= 2;
        num -= 1;
        for (size_t i = 0; i + 1 < bus->route.size(); ++i) {
            auto it = distances_.find({bus->route[i+1], bus->route[i]});
            if (it == distances_.end()) {
                it = distances_.find({bus->route[i], bus->route[i+1]});
            }
            info.real_length += (*it).second;
        }
    }

    info.stops = num;
    info.unique_stops = unique_stop.size();

    return info;
}

vector<string_view> TransportCatalogue::GetBusesForStop(std::string_view name) const {
    Stop* stop = stops_names_.at(name);
    vector<string_view> buses(buses_for_stops_.at(stop).begin(), buses_for_stops_.at(stop).end());
    sort(buses.begin(), buses.end());
    return buses;
}
    
} // namespace TrC