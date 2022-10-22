#pragma once

#include <vector>
#include <deque>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <unordered_map>

#include "domain.h"
#include "svg.h"
#include "map_renderer.h"

namespace TrC {

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddStop(const Stop& stop);
    void AddStop(Stop&& stop);
        
    void AddBus(const Bus& bus);
    void AddBus(Bus&& bus);
        
    void AddDistances(Stop* from, Stop* to, unsigned distance);
    void AddDistances(const std::pair<Stop*,std::vector<detail::DistanceToStop>>& distances);
    void AddDistances(std::pair<Stop*,std::vector<detail::DistanceToStop>>&& distances);
        
    Bus& FindBus(std::string_view name) const;
    Stop& FindStop(std::string_view name) const;
    BusInfo GetBusInfo(std::string_view name) const;
    std::vector<std::string_view> GetBusesForStop(std::string_view name) const;
    
    unsigned StopsDistance(std::pair<Stop*, Stop*> stops) const {
        auto it = distances_.find(stops);
        if (it == distances_.end()) {
            return distances_.at({stops.second, stops.first});
        }
        return it->second;
    }
    
    bool StopCount(std::string_view name) const {
        return stops_names_.count(name);
    }
    
    bool BusCount(std::string_view name) const {
        return buses_names_.count(name);
    }
    
    const std::unordered_map<std::string_view, Bus*>& GetBuses() const {
        return buses_names_;
    }
    
    const std::unordered_map<std::string_view, Stop*>& GetStops() const {
        return stops_names_;
    }
    
    const std::unordered_map<std::pair<Stop*, Stop*>, unsigned, detail::PairHash>& 
        GetDistances() const {
        return distances_;
    }
    
    std::deque<Stop>::const_iterator StopsBegin() const {
        return stops_.cbegin();
    }
    
    std::deque<Stop>::const_iterator StopsEnd() const {
        return stops_.cend();
    }
    
    std::deque<Bus>::const_iterator BusesBegin() const {
        return buses_.cbegin();
    }
    
    std::deque<Bus>::const_iterator BusesEnd() const {
        return buses_.cend();
    }
    
private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stops_names_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> buses_names_;
    std::unordered_map<Stop*, std::unordered_set<std::string_view>> buses_for_stops_;
    std::unordered_map<std::pair<Stop*, Stop*>, unsigned, detail::PairHash> distances_;
};
    
} // namespace TrC