#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <utility>
#include <cmath>

#include "geo.h"

namespace TrC {

struct Stop {
    std::string name;
    geo::Coordinates coord;
};
    
std::istream& operator>>(std::istream& is, Stop& stop);

struct Bus {
    std::string name;        
    std::vector<Stop*> route;
    bool is_ring = false;
};
    
bool operator<(const Bus& lbs, const Bus& rbs);
    
struct BusInfo {
    int stops = 0;
    int unique_stops = 0;
    double length = 0;
    int real_length = 0;
};
    
namespace detail {
    
struct PairHash {
    size_t operator() (const std::pair<Stop*, Stop*>& p) const {
        return p_hash_(p.first) * static_cast<size_t>(std::pow(59, 4)) + p_hash_(p.second) * 59;
    } 
private:
    std::hash<const void*> p_hash_;
};

struct DistanceToStop {
    unsigned int distance;
    std::string stop_name;
};

std::istream& operator>>(std::istream& is, DistanceToStop& dist);
        
} //namespace detail

} //namespace TrC