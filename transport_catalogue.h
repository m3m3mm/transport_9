#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <deque>
#include <unordered_set>
#include "geo.h"

namespace transport_catalogue {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    bool is_roundtrip;
};

struct BusInfo {
    int stops_count;
    int unique_stops_count;
    double route_length;
    double curvature;
};

namespace detail {
    struct PairHash {
        template <class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2>& p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            return h1 ^ h2; // combine hash values
        }
    };
}

class TransportCatalogue {
public:
    void AddStop(const std::string& name, const geo::Coordinates& coordinates);
    void AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip);
    void AddStopDistance(const std::string& stop_name, const std::unordered_map<std::string, int>& distances);
    const Bus* FindBus(const std::string& name) const;
    const Stop* FindStop(const std::string& name) const;
    BusInfo GetBusInfo(const std::string& name) const;
    std::vector<std::string> GetBusesForStop(const std::string& stop_name) const;
    bool HasStop(const std::string& stop_name) const;

private:
    std::unordered_map<std::string, Stop> stops_;
    std::unordered_map<std::string, Bus> buses_;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> stop_distances_;
    std::unordered_map<std::string, std::unordered_set<std::string>> stop_to_buses_;
    // Other private members...
};

} // namespace transport_catalogue
