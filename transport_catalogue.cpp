// transport_catalogue.cpp

#include "transport_catalogue.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace transport_catalogue {

void TransportCatalogue::AddStop(const std::string& name, const geo::Coordinates& coordinates) {
    stops_.push_back({name, coordinates});
    const Stop* stop_ptr = &stops_.back();
    stopname_to_stop_[name] = stop_ptr;
}


void TransportCatalogue::AddStopDistance(const std::string& stop_name, const std::unordered_map<std::string, int>& distances) {
    const Stop* from_stop = FindStop(stop_name);
    if (!from_stop) return;

    for (const auto& [to_name, distance] : distances) {
        const Stop* to_stop = FindStop(to_name);
        if (!to_stop) continue;

        distances_[{from_stop, to_stop}] = distance;
    }
}

const Bus* TransportCatalogue::FindBus(const std::string& name) const {
    auto it = busname_to_bus_.find(name);
    return it != busname_to_bus_.end() ? it->second : nullptr;
}

const Stop* TransportCatalogue::FindStop(const std::string& name) const {
    auto it = stopname_to_stop_.find(name);
    return it != stopname_to_stop_.end() ? it->second : nullptr;
}

std::vector<std::string> TransportCatalogue::GetBusesForStop(const std::string& stop_name) const {
    const Stop* stop = FindStop(stop_name);
    if (!stop) return {};

    auto it = stop_to_buses_.find(stop);
    if (it != stop_to_buses_.end()) {
        return std::vector<std::string>(it->second.begin(), it->second.end());
    }
    return {};
}

BusInfo TransportCatalogue::GetBusInfo(const std::string& name) const {
    const Bus* bus = FindBus(name);
    if (!bus) {
        return {0, 0, 0.0, 0.0};
    }

    int stops_count = bus->is_roundtrip ? bus->stops.size() : bus->stops.size() * 2 - 1;
    std::unordered_set<const Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
    int unique_stops_count = unique_stops.size();

    double route_length = 0.0;
    double geo_route_length = 0.0;

    auto compute_distance = [this](const Stop* from, const Stop* to) {
        auto it = distances_.find({from, to});
        if (it != distances_.end()) {
            return it->second;
        }
        it = distances_.find({to, from});
        if (it != distances_.end()) {
            return it->second;  // Use the reverse distance if available
        }
        // If no distance is found in either direction, use geographic distance
        return static_cast<int>(geo::ComputeDistance(from->coordinates, to->coordinates));
    };

    for (size_t i = 1; i < bus->stops.size(); ++i) {
        const Stop* from = bus->stops[i - 1];
        const Stop* to = bus->stops[i];

        route_length += compute_distance(from, to);
        geo_route_length += geo::ComputeDistance(from->coordinates, to->coordinates);
    }

    // For non-roundtrip routes, add the return journey
    if (!bus->is_roundtrip && bus->stops.size() > 1) {
        for (size_t i = bus->stops.size() - 1; i > 0; --i) {
            const Stop* from = bus->stops[i];
            const Stop* to = bus->stops[i - 1];

            route_length += compute_distance(from, to);
            geo_route_length += geo::ComputeDistance(from->coordinates, to->coordinates);
        }
    }

    double curvature = route_length / geo_route_length;

    return {stops_count, unique_stops_count, route_length, curvature};
}

void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip) {
    buses_.push_back({name, {}, is_roundtrip});
    Bus& bus = buses_.back();
    for (const auto& stop_name : stops) {
        const Stop* stop = FindStop(stop_name);
        if (stop) {
            bus.stops.push_back(stop);
            stop_to_buses_[stop].insert(name);
        }
    }
    busname_to_bus_[name] = &bus;
}

bool TransportCatalogue::HasStop(const std::string& stop_name) const {
    return stopname_to_stop_.find(stop_name) != stopname_to_stop_.end();
}

} // namespace transport_catalogue
