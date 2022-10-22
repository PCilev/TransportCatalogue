#include "domain.h"

namespace TrC {
    
using namespace std;
    
namespace detail {

std::istream& operator>>(std::istream& is, DistanceToStop& dist) {
    char ch;
    string s;
    string name;
    is >> dist.distance >> ch >> s;
    getline(is, name);
    dist.stop_name = move(name.substr(name.find_first_not_of(' ')));
    return is;
}
        
} // namespace detail

bool operator<(const Bus& lbs, const Bus& rbs) {
    return lbs.name < rbs.name;
}
    
istream& operator>>(istream& is, Stop& stop) {
    char ch = ':';
    getline(is, stop.name, ch);
    is >> stop.coord.lat >> ch >> stop.coord.lng;
    return is;
}
    
} // namespace TrC