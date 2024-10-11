#include "transport_catalogue.h"
#include <algorithm>
#include <cmath>

namespace transport_catalogue {

void TransportCatalogue::AddStop(const std::string& name, const geo::Coordinates& coordinates) {
    stops_.emplace(name, Stop{name, coordinates});
}

void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip) {
    buses_.emplace(name, Bus{name, {}, is_roundtrip});
    auto& bus = buses_.at(name);
    for (const auto& stop_name : stops) {
        const Stop* stop = FindStop(stop_name);
        if (stop) {
            bus.stops.push_back(stop);
            stop_to_buses_[stop_name].insert(name);
        }
    }
}

void TransportCatalogue::AddStopDistance(const std::string& stop_name, const std::unordered_map<std::string, int>& distances) {
    for (const auto& [neighbor, distance] : distances) {
        stop_distances_[stop_name][neighbor] = distance;
    }
}

const Bus* TransportCatalogue::FindBus(const std::string& name) const {
    auto it = buses_.find(name);
    return it != buses_.end() ? &it->second : nullptr;
}

const Stop* TransportCatalogue::FindStop(const std::string& name) const {
    auto it = stops_.find(name);
    return it != stops_.end() ? &it->second : nullptr;
}

std::vector<std::string> TransportCatalogue::GetBusesForStop(const std::string& stop_name) const {
    auto it = stop_to_buses_.find(stop_name);
    return it != stop_to_buses_.end() ? std::vector<std::string>(it->second.begin(), it->second.end()) : std::vector<std::string>();
}

BusInfo TransportCatalogue::GetBusInfo(const std::string& name) const {
    const Bus* bus = FindBus(name);
    if (!bus) {
        return {0, 0, 0.0, 0.0};
    }

    int stops_count = bus->is_roundtrip ? bus->stops.size() : bus->stops.size() * 2 - 1;
    std::unordered_set<const Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
    int unique_stops_count = unique_stops.size();

    double geo_length = 0.0;
    double road_length = 0.0;

    for (size_t i = 1; i < bus->stops.size(); ++i) {
        geo_length += geo::ComputeDistance(bus->stops[i - 1]->coordinates, bus->stops[i]->coordinates);
        
        // Проверяем наличие ключа перед вызовом at()
        const std::string& from = bus->stops[i - 1]->name;
        const std::string& to = bus->stops[i]->name;
        
        if (stop_distances_.count(from) && stop_distances_.at(from).count(to)) {
            road_length += stop_distances_.at(from).at(to);
        } else {
            // Обработка случая, когда расстояние между остановками не указано
            road_length += geo::ComputeDistance(bus->stops[i - 1]->coordinates, bus->stops[i]->coordinates);
        }
    }

    if (!bus->is_roundtrip && bus->stops.size() > 1) {
        for (size_t i = bus->stops.size() - 1; i > 0; --i) {
            geo_length += geo::ComputeDistance(bus->stops[i]->coordinates, bus->stops[i - 1]->coordinates);
            
            const std::string& from = bus->stops[i]->name;
            const std::string& to = bus->stops[i - 1]->name;
            
            if (stop_distances_.count(from) && stop_distances_.at(from).count(to)) {
                road_length += stop_distances_.at(from).at(to);
            } else {
                road_length += geo::ComputeDistance(bus->stops[i]->coordinates, bus->stops[i - 1]->coordinates);
            }
        }
    }

    double curvature = road_length / geo_length;

    return {stops_count, unique_stops_count, road_length, curvature};
}

bool TransportCatalogue::HasStop(const std::string& stop_name) const {
    return stops_.find(stop_name) != stops_.end();
}

} // namespace transport_catalogue
