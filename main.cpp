#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

struct CampusNode {
    std::string id;
    std::string name;
    std::string type;
    double latitude;
    double longitude;
};

struct CampusEdge {
    std::size_t destination;
    double distance;
};

struct Route {
    std::vector<std::size_t> nodeIndexes;
    double distance;
};

std::string trim(const std::string& value) {
    const std::size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const std::size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::vector<std::string> parseCsvRow(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool quoted = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char character = line[i];
        if (character == '"') {
            if (quoted && i + 1 < line.size() && line[i + 1] == '"') {
                field += '"';
                ++i;
            } else {
                quoted = !quoted;
            }
        } else if (character == ',' && !quoted) {
            fields.push_back(trim(field));
            field.clear();
        } else {
            field += character;
        }
    }

    if (quoted) throw std::runtime_error("CSV row contains an unclosed quote.");
    fields.push_back(trim(field));
    return fields;
}

std::size_t findColumn(
    const std::vector<std::string>& headers,
    const std::string& name,
    const std::string& filename
) {
    const auto found = std::find(headers.begin(), headers.end(), name);
    if (found == headers.end()) {
        throw std::runtime_error(filename + " is missing the " + name + " column.");
    }
    return static_cast<std::size_t>(found - headers.begin());
}

const std::string& fieldAt(
    const std::vector<std::string>& fields,
    std::size_t index,
    const std::string& filename,
    std::size_t lineNumber
) {
    if (index >= fields.size()) {
        throw std::runtime_error(
            filename + " has an incomplete row at line " + std::to_string(lineNumber) + "."
        );
    }
    return fields[index];
}

double parseNumber(
    const std::string& value,
    const std::string& filename,
    std::size_t lineNumber
) {
    std::size_t parsedLength = 0;
    double number;
    try {
        number = std::stod(value, &parsedLength);
    } catch (const std::exception&) {
        throw std::runtime_error(
            filename + " has an invalid number at line " + std::to_string(lineNumber) + "."
        );
    }
    if (parsedLength != value.size() || !std::isfinite(number)) {
        throw std::runtime_error(
            filename + " has an invalid number at line " + std::to_string(lineNumber) + "."
        );
    }
    return number;
}

class CampusGraph {
private:
    std::vector<CampusNode> nodes;
    std::vector<std::vector<CampusEdge>> adjacency;
    std::unordered_map<std::string, std::size_t> nodeIndexes;

    std::size_t findNode(const std::string& id) const {
        const auto found = nodeIndexes.find(id);
        if (found == nodeIndexes.end()) {
            throw std::runtime_error("Unknown campus location ID: " + id);
        }
        return found->second;
    }

public:
    void loadNodes(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) throw std::runtime_error("Could not open " + filename + ".");

        std::string line;
        if (!std::getline(file, line)) {
            throw std::runtime_error(filename + " is empty.");
        }
        const auto headers = parseCsvRow(line);
        const std::size_t idColumn = findColumn(headers, "node_id", filename);
        const std::size_t nameColumn = findColumn(headers, "name", filename);
        const std::size_t typeColumn = findColumn(headers, "node_type", filename);
        const std::size_t latColumn = findColumn(headers, "lat", filename);
        const std::size_t lonColumn = findColumn(headers, "lon", filename);

        std::size_t lineNumber = 1;
        while (std::getline(file, line)) {
            ++lineNumber;
            if (trim(line).empty()) continue;
            const auto fields = parseCsvRow(line);
            CampusNode node{
                fieldAt(fields, idColumn, filename, lineNumber),
                fieldAt(fields, nameColumn, filename, lineNumber),
                fieldAt(fields, typeColumn, filename, lineNumber),
                parseNumber(fieldAt(fields, latColumn, filename, lineNumber), filename, lineNumber),
                parseNumber(fieldAt(fields, lonColumn, filename, lineNumber), filename, lineNumber)
            };
            if (node.id.empty() || node.name.empty()) {
                throw std::runtime_error(
                    filename + " has a location without an ID or name at line " +
                    std::to_string(lineNumber) + "."
                );
            }
            const std::size_t index = nodes.size();
            if (!nodeIndexes.emplace(node.id, index).second) {
                throw std::runtime_error(filename + " has a duplicate node ID: " + node.id);
            }
            nodes.push_back(node);
        }
        if (nodes.empty()) throw std::runtime_error(filename + " contains no locations.");
        adjacency.resize(nodes.size());
    }

    void loadEdges(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) throw std::runtime_error("Could not open " + filename + ".");

        std::string line;
        if (!std::getline(file, line)) {
            throw std::runtime_error(filename + " is empty.");
        }
        const auto headers = parseCsvRow(line);
        const std::size_t fromColumn = findColumn(headers, "node_from", filename);
        const std::size_t toColumn = findColumn(headers, "node_to", filename);
        const std::size_t distanceColumn = findColumn(headers, "edge_cost_m", filename);

        std::size_t lineNumber = 1;
        while (std::getline(file, line)) {
            ++lineNumber;
            if (trim(line).empty()) continue;
            const auto fields = parseCsvRow(line);
            const std::string& fromId = fieldAt(fields, fromColumn, filename, lineNumber);
            const std::string& toId = fieldAt(fields, toColumn, filename, lineNumber);
            const double distance = parseNumber(
                fieldAt(fields, distanceColumn, filename, lineNumber), filename, lineNumber
            );
            if (distance < 0) {
                throw std::runtime_error(
                    filename + " has a negative edge distance at line " +
                    std::to_string(lineNumber) + "."
                );
            }
            const std::size_t from = findNode(fromId);
            const std::size_t to = findNode(toId);
            adjacency[from].push_back({to, distance});
            adjacency[to].push_back({from, distance});
        }
    }

    std::size_t nodeCount() const { return nodes.size(); }

    void listPlaces() const {
        for (const CampusNode& node : nodes) {
            if (node.type == "circle node") {
                std::cout << node.id << "\t" << node.name << '\n';
            }
        }
    }

    bool shortestRoute(const std::string& fromId, const std::string& toId, Route& route) const {
        const std::size_t start = findNode(fromId);
        const std::size_t destination = findNode(toId);
        const double infinity = std::numeric_limits<double>::infinity();
        std::vector<double> distances(nodes.size(), infinity);
        std::vector<std::size_t> previous(nodes.size(), nodes.size());
        using QueueEntry = std::pair<double, std::size_t>;
        std::priority_queue<
            QueueEntry,
            std::vector<QueueEntry>,
            std::greater<QueueEntry>
        > queue;

        distances[start] = 0;
        queue.push({0, start});
        while (!queue.empty()) {
            const QueueEntry current = queue.top();
            queue.pop();
            if (current.first != distances[current.second]) continue;
            if (current.second == destination) break;

            for (const CampusEdge& edge : adjacency[current.second]) {
                const double candidate = current.first + edge.distance;
                if (candidate < distances[edge.destination]) {
                    distances[edge.destination] = candidate;
                    previous[edge.destination] = current.second;
                    queue.push({candidate, edge.destination});
                }
            }
        }

        if (!std::isfinite(distances[destination])) return false;
        route.distance = distances[destination];
        route.nodeIndexes.clear();
        for (std::size_t node = destination; node != nodes.size(); node = previous[node]) {
            route.nodeIndexes.push_back(node);
            if (node == start) break;
        }
        if (route.nodeIndexes.back() != start) return false;
        std::reverse(route.nodeIndexes.begin(), route.nodeIndexes.end());
        return true;
    }

    const CampusNode& getNode(std::size_t index) const { return nodes.at(index); }
};

}  // namespace

int main(int argc, char* argv[]) {
    try {
        CampusGraph campus;
        campus.loadNodes("data/campus_node_coordinates.csv");
        campus.loadEdges("data/campus_edges.csv");
        std::cout << "Loaded " << campus.nodeCount()
                  << " campus locations from CSV data.\n";

        if (argc == 1 || (argc == 2 && std::string(argv[1]) == "--places")) {
            std::cout << "Usage: graph.exe --route <start-id> <destination-id>\n"
                      << "Mapped campus places:\n";
            campus.listPlaces();
            return 0;
        }
        if (argc != 4 || std::string(argv[1]) != "--route") {
            std::cerr << "Usage: graph.exe --route <start-id> <destination-id>\n"
                      << "       graph.exe --places\n";
            return 2;
        }

        Route route;
        if (!campus.shortestRoute(argv[2], argv[3], route)) {
            std::cout << "No route is available between those locations.\n";
            return 1;
        }
        std::cout << std::fixed << std::setprecision(1)
                  << "Shortest walking route (" << route.distance << " m, "
                  << static_cast<int>(std::ceil(route.distance / 75.0)) << " min):\n";
        for (std::size_t i = 0; i < route.nodeIndexes.size(); ++i) {
            const CampusNode& node = campus.getNode(route.nodeIndexes[i]);
            if (i != 0) std::cout << " -> ";
            std::cout << node.name;
        }
        std::cout << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
