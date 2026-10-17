#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

const int MAX_LOCATIONS = 200;
const int MAX_BUILDINGS = 20;

enum LocationType {
    ROOM, CORRIDOR, STAIRCASE, LIFT, ENTRANCE
};

string locationTypeToString(LocationType type) {
    switch (type) {
        case ROOM: return "Room";
        case CORRIDOR: return "Corridor";
        case STAIRCASE: return "Staircase";
        case LIFT: return "Lift";
        case ENTRANCE: return "Entrance";
        default: return "Unknown";
    }
}

LocationType stringToLocationType(const string& type) {
    if (type == "ROOM") return ROOM;
    if (type == "CORRIDOR") return CORRIDOR;
    if (type == "STAIRCASE") return STAIRCASE;
    if (type == "LIFT") return LIFT;
    if (type == "ENTRANCE") return ENTRANCE;
    throw runtime_error("Unknown location type: " + type);
}

struct Edge {
    int destination;
    int distance;
};

struct Building {
    int id;
    string name;
};

struct Location {
    int id;
    string name;
    int buildingID;
    float x, y, z;  // z represents the floor
    LocationType type;
};

class Graph {
private:
    vector<Building> buildings;
    vector<Location> locations;
    vector<vector<Edge>> adjList;

public:
    void addBuilding(int id, const string& name) {
        if (id < 0 || id >= MAX_BUILDINGS)
            throw runtime_error("Building ID is outside the supported range.");
        if (id != static_cast<int>(buildings.size()))
            throw runtime_error("Building IDs must be entered in order starting from 0.");
        buildings.push_back({id, name});
    }

    void addLocation(int id, const string& name, int buildingID,
                     LocationType type, float x, float y, float z) {
        if (id < 0 || id >= MAX_LOCATIONS)
            throw runtime_error("Location ID is outside the supported range.");
        if (id != static_cast<int>(locations.size()))
            throw runtime_error("Location IDs must be entered in order starting from 0.");
        if (buildingID < 0 || buildingID >= static_cast<int>(buildings.size()))
            throw runtime_error("Location refers to a building that has not been added.");
        locations.push_back({id, name, buildingID, x, y, z, type});
        adjList.emplace_back();
    }

    void addPath(int source, int destination, int distance) {
        if (source < 0 || destination < 0 ||
            source >= static_cast<int>(locations.size()) ||
            destination >= static_cast<int>(locations.size())) {
            throw runtime_error("Path refers to a location ID that does not exist.");
        }
        if (distance < 0)
            throw runtime_error("Distance cannot be negative.");

        // Add both directions because the path is assumed to be two-way.
        adjList[source].push_back({destination, distance});
        adjList[destination].push_back({source, distance});
    }

    void displayBuildings() const {
        cout << "\n              BUILDINGS\n";
        for (const auto& building : buildings) {
            cout << "\nID   : " << building.id
                 << "\nName : " << building.name << "\n";
        }
    }

    void displayLocations() const {
        cout << "\nCAMPUS LOCATIONS\n";
        for (const auto& location : locations) {
            cout << "\n----------------------------------------"
                 << "\nID       : " << location.id
                 << "\nName     : " << location.name
                 << "\nBuilding : " << buildings[location.buildingID].name
                 << "\nType     : " << locationTypeToString(location.type)
                 << "\nCoordinates : (" << location.x << ", "
                 << location.y << ", " << location.z << ")\n";
        }
    }

    void displayGraph() const {
        cout << "\nADJACENCY LIST\n";
        for (size_t i = 0; i < locations.size(); ++i) {
            cout << "\n" << locations[i].name;
            for (const auto& edge : adjList[i]) {
                cout << " -> " << locations[edge.destination].name
                     << " (" << edge.distance << "m)";
            }
            cout << "\n";
        }
    }
};

vector<string> splitCSVLine(const string& line) {
    // This simple CSV reader expects fields without commas inside them.
    vector<string> fields;
    string field;
    stringstream ss(line);
    while (getline(ss, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

void loadBuildings(Graph& graph, const string& filename) {
    ifstream file(filename);
    if (!file) throw runtime_error("Could not open " + filename);

    string line;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        if (line.empty()) continue;
        auto fields = splitCSVLine(line);
        if (fields.size() != 2) throw runtime_error("Invalid row in " + filename);
        graph.addBuilding(stoi(fields[0]), fields[1]);
    }
}

void loadLocations(Graph& graph, const string& filename) {
    ifstream file(filename);
    if (!file) throw runtime_error("Could not open " + filename);

    string line;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        if (line.empty()) continue;
        auto fields = splitCSVLine(line);
        if (fields.size() != 7) throw runtime_error("Invalid row in " + filename);
        graph.addLocation(
            stoi(fields[0]), fields[1], stoi(fields[2]),
            stringToLocationType(fields[3]),
            stof(fields[4]), stof(fields[5]), stof(fields[6])
        );
    }
}

void loadPaths(Graph& graph, const string& filename) {
    ifstream file(filename);
    if (!file) throw runtime_error("Could not open " + filename);

    string line;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        if (line.empty()) continue;
        auto fields = splitCSVLine(line);
        if (fields.size() != 3) throw runtime_error("Invalid row in " + filename);
        graph.addPath(stoi(fields[0]), stoi(fields[1]), stoi(fields[2]));
    }
}

void loadFromCSV(Graph& graph) {
    // Load in this order: buildings, locations, then paths.
    loadBuildings(graph, "buildings.csv");
    loadLocations(graph, "locations.csv");
    loadPaths(graph, "paths.csv");
}

void loadDemoData(Graph& graph) {
    // Demo data matching the original C program.
    graph.addBuilding(0, "CSE Building");
    graph.addBuilding(1, "Main Academic Block");
    graph.addBuilding(2, "Library");
    graph.addBuilding(3, "Administrative Block");

    graph.addLocation(0, "CSE Entrance", 0, ENTRANCE, 0.0, 0.0, 0.0);
    graph.addLocation(1, "CR15", 0, ROOM, 0.0, 0.0, 0.0);
    graph.addLocation(2, "CR16", 0, ROOM, 0.0, 0.0, 0.0);
    graph.addLocation(3, "Staircase", 0, STAIRCASE, 0.0, 0.0, 0.0);
    graph.addLocation(4, "LT12", 0, ROOM, 0.0, 0.0, 0.0);
    graph.addLocation(5, "LT13", 0, ROOM, 0.0, 0.0, 0.0);
    graph.addLocation(6, "Academic Entrance", 1, ENTRANCE, 0.0, 0.0, 0.0);
    graph.addLocation(7, "CR20", 1, ROOM, 0.0, 0.0, 0.0);
    graph.addLocation(8, "Library Entrance", 2, ENTRANCE, 0.0, 0.0, 0.0);
    graph.addLocation(9, "Admin Entrance", 3, ENTRANCE, 0.0, 0.0, 0.0);

    graph.addPath(0, 1, 10);
    graph.addPath(1, 3, 15);
    graph.addPath(3, 2, 10);
    graph.addPath(3, 4, 12);
    graph.addPath(4, 5, 10);
    graph.addPath(0, 6, 150);
    graph.addPath(6, 7, 20);
    graph.addPath(6, 8, 100);
    graph.addPath(6, 9, 200);
}

int main(int argc, char* argv[]) {
    Graph campus;

    try {
        if (argc > 1 && string(argv[1]) == "--demo") {
            loadDemoData(campus);
            cout << "Loaded built-in demo data.\n";
        } else {
            loadFromCSV(campus);
            cout << "Loaded campus data from CSV files.\n";
        }

        campus.displayBuildings();
        campus.displayLocations();
        campus.displayGraph();
    } catch (const exception& error) {
        cerr << "\nError: " << error.what() << "\n";
        cerr << "Put buildings.csv, locations.csv, and paths.csv beside the program, "
                "or run with --demo.\n";
        return 1;
    }

    return 0;
}
