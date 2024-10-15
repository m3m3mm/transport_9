// input_reader.cpp

#include "input_reader.h"
#include <algorithm>
#include <cassert>
#include <iterator>

namespace transport_catalogue {
namespace input {
namespace detail {

geo::Coordinates ParseCoordinates(std::string_view str) {
    size_t comma = str.find(',');
    if (comma == str.npos) {
        return {0, 0};
    }

    double lat = std::stod(std::string(str.substr(0, comma)));
    double lng = std::stod(std::string(str.substr(comma + 1)));

    return {lat, lng};
}

std::unordered_map<std::string, int> ParseDistances(std::string_view str) {
    std::unordered_map<std::string, int> distances;
    size_t pos = 0;
    while (pos < str.size()) {
        size_t next_comma = str.find(',', pos);
        if (next_comma == std::string::npos) next_comma = str.size();

        size_t m_pos = str.rfind('m', next_comma);
        if (m_pos == std::string::npos || m_pos <= pos) break;

        int distance = std::stoi(std::string(str.substr(pos, m_pos - pos)));
        size_t to_pos = str.rfind("to", m_pos);
        if (to_pos == std::string::npos || to_pos <= pos) break;

        std::string stop_name = std::string(str.substr(to_pos + 2, m_pos - to_pos - 2));
        stop_name = stop_name.substr(stop_name.find_first_not_of(" "));
        stop_name = stop_name.substr(0, stop_name.find_last_not_of(" ") + 1);

        distances[stop_name] = distance;
        pos = next_comma + 1;
    }
    return distances;
}

std::vector<std::string> ParseRoute(std::string_view route) {
    std::vector<std::string> stops;
    size_t pos = 0;
    while (pos < route.length()) {
        size_t next = route.find_first_of(">-", pos);
        if (next == std::string_view::npos) {
            next = route.length();
        }
        std::string_view stop = route.substr(pos, next - pos);
        stop = stop.substr(stop.find_first_not_of(' '));
        stop = stop.substr(0, stop.find_last_not_of(' ') + 1);
        stops.push_back(std::string(stop));
        pos = next + 1;
    }
    return stops;
}

} // namespace detail

void InputReader::ParseLine(std::string_view line) {
    size_t colon = line.find(':');
    if (colon == line.npos) {
        return;
    }

    std::string_view command_part = line.substr(0, colon);
    size_t space = command_part.find(' ');
    if (space == command_part.npos) {
        return;
    }

    commands_.push_back({
        std::string(command_part.substr(0, space)),
        std::string(command_part.substr(space + 1)),
        std::string(line.substr(colon + 2))
    });
}

void InputReader::ApplyCommands(TransportCatalogue& catalogue) const {
    // First pass: add stops
    for (const auto& command : commands_) {
        if (command.command == "Stop") {
            auto [lat, lng] = detail::ParseCoordinates(command.description);
            catalogue.AddStop(command.id, {lat, lng});
        }
    }

    // Second pass: add distances and buses
    for (const auto& command : commands_) {
        if (command.command == "Stop") {
            auto distances = detail::ParseDistances(command.description);
            catalogue.AddStopDistance(command.id, distances);
        } else if (command.command == "Bus") {
            auto stops = detail::ParseRoute(command.description);
            bool is_roundtrip = command.description.find('>') != std::string::npos;
            catalogue.AddBus(command.id, stops, is_roundtrip);
        }
    }
}

} // namespace input
} // namespace transport_catalogue
