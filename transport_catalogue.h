

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
    struct StopPair {
        const Stop* from;
        const Stop* to;

        bool operator==(const StopPair& other) const {
            return from == other.from && to == other.to;
        }
    };

    struct StopPairHasher {
        std::size_t operator()(const StopPair& pair) const {
            const std::size_t h1 = std::hash<const Stop*>{}(pair.from);
            const std::size_t h2 = std::hash<const Stop*>{}(pair.to);
            return h1 ^ (h2 << 1);
        }
    };

    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string, const Bus*> busname_to_bus_;
    std::unordered_map<const Stop*, std::set<std::string>> stop_to_buses_;
    std::unordered_map<StopPair, int, StopPairHasher> distances_;

};

} // namespace transport_catalogue
