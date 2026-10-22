#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "LiveClock.h"
#include "User.h"

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

bool parseInteger(const std::string& value, int& result) {
    if (value.empty()) return false;
    std::size_t parsedLength = 0;
    try {
        result = std::stoi(value, &parsedLength);
    } catch (const std::exception&) {
        return false;
    }
    return parsedLength == value.size();
}

bool parseTime(const std::string& value, int& hour, int& minute) {
    if (value.size() != 5 || value[2] != ':') return false;
    int parsedHour = 0;
    int parsedMinute = 0;
    if (!parseInteger(value.substr(0, 2), parsedHour) ||
        !parseInteger(value.substr(3, 2), parsedMinute) ||
        parsedHour < 0 || parsedHour > 23 ||
        parsedMinute < 0 || parsedMinute > 59) {
        return false;
    }
    hour = parsedHour;
    minute = parsedMinute;
    return true;
}

bool parseDate(const std::string& value) {
    if (value.size() != 10 || value[4] != '-' || value[7] != '-') return false;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (value[i] < '0' || value[i] > '9') return false;
    }
    int year = 0;
    int month = 0;
    int day = 0;
    if (!parseInteger(value.substr(0, 4), year) ||
        !parseInteger(value.substr(5, 2), month) ||
        !parseInteger(value.substr(8, 2), day) ||
        year < 1 || month < 1 || month > 12) {
        return false;
    }
    const bool leapYear =
        year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
    static const int DAYS_IN_MONTH[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    const int daysInMonth = month == 2 && leapYear
        ? 29
        : DAYS_IN_MONTH[month - 1];
    return day >= 1 && day <= daysInMonth;
}

std::string normalizeDay(const std::string& value) {
    std::string day = value;
    std::transform(day.begin(), day.end(), day.begin(),
                   [](unsigned char character) {
                       return static_cast<char>(std::toupper(character));
                   });
    static const char* DAYS[] = {
        "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"
    };
    for (const char* validDay : DAYS) {
        if (day == validDay) return day;
    }
    return "";
}

GroupScope parseGroupScope(const std::string& value) {
    if (value == "NONE") return GroupScope::NONE;
    if (value == "G1") return GroupScope::G1;
    if (value == "G2") return GroupScope::G2;
    if (value == "BOTH") return GroupScope::BOTH;
    throw std::runtime_error("Invalid timetable group scope: " + value);
}

std::string groupScopeName(GroupScope scope) {
    switch (scope) {
        case GroupScope::NONE: return "ALL";
        case GroupScope::G1: return "G1";
        case GroupScope::G2: return "G2";
        case GroupScope::BOTH: return "ALL";
    }
    throw std::runtime_error("Unknown timetable group scope.");
}

TimeSlot parseTimeSlot(
    const std::string& dayValue,
    const std::string& startValue,
    const std::string& endValue,
    const std::string& filename,
    std::size_t lineNumber
) {
    const std::string day = normalizeDay(dayValue);
    int startHour = 0;
    int startMinute = 0;
    int endHour = 0;
    int endMinute = 0;
    if (day.empty() ||
        !parseTime(startValue, startHour, startMinute) ||
        !parseTime(endValue, endHour, endMinute) ||
        endHour * 60 + endMinute <= startHour * 60 + startMinute) {
        throw std::runtime_error(
            filename + " has an invalid day or time range at line " +
            std::to_string(lineNumber) + "."
        );
    }
    return TimeSlot(day, startHour, startMinute, endHour, endMinute);
}

bool parseBoolean(const std::string& value, bool& result) {
    if (value == "true" || value == "1") {
        result = true;
        return true;
    }
    if (value == "false" || value == "0") {
        result = false;
        return true;
    }
    return false;
}

struct TimetableColumns {
    std::size_t recordType;
    std::size_t section;
    std::size_t semester;
    std::size_t day;
    std::size_t startTime;
    std::size_t endTime;
    std::size_t courseCode;
    std::size_t subjectName;
    std::size_t facultyName;
    std::size_t subjectType;
    std::size_t venue;
    std::size_t groupScope;
    std::size_t date;
    std::size_t cancelled;
    std::size_t reason;
    std::size_t isExample;
};

TimetableColumns findTimetableColumns(
    const std::vector<std::string>& headers,
    const std::string& filename
) {
    return {
        findColumn(headers, "record_type", filename),
        findColumn(headers, "section", filename),
        findColumn(headers, "semester", filename),
        findColumn(headers, "day", filename),
        findColumn(headers, "start_time", filename),
        findColumn(headers, "end_time", filename),
        findColumn(headers, "course_code", filename),
        findColumn(headers, "subject_name", filename),
        findColumn(headers, "faculty_name", filename),
        findColumn(headers, "subject_type", filename),
        findColumn(headers, "venue", filename),
        findColumn(headers, "group_scope", filename),
        findColumn(headers, "date", filename),
        findColumn(headers, "cancelled", filename),
        findColumn(headers, "reason", filename),
        findColumn(headers, "is_example", filename)
    };
}

bool loadTimetable(
    const std::string& filename,
    const std::string& sectionName,
    int semester,
    Section& section,
    Admin& admin
) {
    std::ifstream file(filename);
    if (!file) throw std::runtime_error("Could not open " + filename + ".");

    std::string line;
    if (!std::getline(file, line)) {
        throw std::runtime_error(filename + " is empty.");
    }
    const TimetableColumns columns =
        findTimetableColumns(parseCsvRow(line), filename);

    std::vector<std::vector<std::string>> matchingRows;
    std::vector<std::size_t> lineNumbers;
    std::size_t lineNumber = 1;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (trim(line).empty()) continue;
        const std::vector<std::string> fields = parseCsvRow(line);
        const std::string& rowSection =
            fieldAt(fields, columns.section, filename, lineNumber);
        int rowSemester = 0;
        if (!parseInteger(fieldAt(fields, columns.semester, filename, lineNumber),
                          rowSemester) || rowSemester < 1) {
            throw std::runtime_error(
                filename + " has an invalid semester at line " +
                std::to_string(lineNumber) + "."
            );
        }
        if (rowSection == sectionName && rowSemester == semester) {
            matchingRows.push_back(fields);
            lineNumbers.push_back(lineNumber);
        }
    }
    if (file.bad()) throw std::runtime_error("Failed while reading " + filename + ".");
    if (matchingRows.empty()) {
        throw std::runtime_error(
            "No timetable records found for section " + sectionName +
            ", semester " + std::to_string(semester) + "."
        );
    }

    bool hasExampleData = false;
    for (std::size_t i = 0; i < matchingRows.size(); ++i) {
        const std::vector<std::string>& fields = matchingRows[i];
        const std::size_t rowLine = lineNumbers[i];
        bool isExample = false;
        if (!parseBoolean(
                fieldAt(fields, columns.isExample, filename, rowLine), isExample
            )) {
            throw std::runtime_error(
                filename + " has an invalid is_example value at line " +
                std::to_string(rowLine) + "."
            );
        }
        hasExampleData = hasExampleData || isExample;
        const std::string& recordType =
            fieldAt(fields, columns.recordType, filename, rowLine);
        const std::string& day =
            fieldAt(fields, columns.day, filename, rowLine);
        const std::string& startTime =
            fieldAt(fields, columns.startTime, filename, rowLine);
        const std::string& endTime =
            fieldAt(fields, columns.endTime, filename, rowLine);
        const TimeSlot slot =
            parseTimeSlot(day, startTime, endTime, filename, rowLine);
        const GroupScope scope =
            parseGroupScope(fieldAt(fields, columns.groupScope, filename, rowLine));

        if (recordType == "class") {
            const std::string& subjectType =
                fieldAt(fields, columns.subjectType, filename, rowLine);
            std::shared_ptr<Subject> subject;
            const std::string& courseCode =
                fieldAt(fields, columns.courseCode, filename, rowLine);
            const std::string& subjectName =
                fieldAt(fields, columns.subjectName, filename, rowLine);
            const std::string& facultyName =
                fieldAt(fields, columns.facultyName, filename, rowLine);
            if (courseCode.empty() || subjectName.empty() ||
                facultyName.empty() ||
                (subjectType != "THEORY" && subjectType != "PRACTICAL")) {
                throw std::runtime_error(
                    filename + " has an invalid class record at line " +
                    std::to_string(rowLine) + "."
                );
            }
            if (subjectType == "PRACTICAL") {
                subject = std::make_shared<PracticalSubject>(
                    courseCode, subjectName, facultyName, semester
                );
            } else {
                subject = std::make_shared<TheorySubject>(
                    courseCode, subjectName, facultyName, semester
                );
            }
            const ScheduleEntry entry(
                slot, subject,
                Venue(fieldAt(fields, columns.venue, filename, rowLine)),
                scope
            );
            if (entry.getVenue().getName().empty()) {
                throw std::runtime_error(
                    filename + " has a class without a venue at line " +
                    std::to_string(rowLine) + "."
                );
            }
            if (!admin.addClassEntry(section, semester, entry, "loaded from CSV")) {
                throw std::runtime_error(
                    filename + " contains a class clash at line " +
                    std::to_string(rowLine) + "."
                );
            }
        } else if (recordType == "override") {
            const std::string& date = fieldAt(fields, columns.date, filename, rowLine);
            bool cancelled = false;
            if (!parseDate(date) ||
                !parseBoolean(fieldAt(fields, columns.cancelled, filename, rowLine),
                              cancelled) ||
                fieldAt(fields, columns.reason, filename, rowLine).empty()) {
                throw std::runtime_error(
                    filename + " has an invalid override at line " +
                    std::to_string(rowLine) + "."
                );
            }
            const std::vector<ScheduleEntry> dayEntries =
                section.getTimetable(semester)->getEntriesForDay(slot.getDay());
            const bool hasMatchingClass = std::any_of(
                dayEntries.begin(), dayEntries.end(),
                [&](const ScheduleEntry& entry) {
                    return entry.getTimeSlot() == slot;
                }
            );
            if (!hasMatchingClass) {
                throw std::runtime_error(
                    filename + " has an override without a matching class at line " +
                    std::to_string(rowLine) + "."
                );
            }
            const std::string& venue =
                fieldAt(fields, columns.venue, filename, rowLine);
            if ((cancelled && !venue.empty()) || (!cancelled && venue.empty())) {
                throw std::runtime_error(
                    filename + " has an invalid override venue at line " +
                    std::to_string(rowLine) + "."
                );
            }
            admin.addTemporaryOverride(
                section, semester,
                TemporaryOverride(
                    date, slot, cancelled, Venue(venue),
                    fieldAt(fields, columns.reason, filename, rowLine)
                ),
                "loaded from CSV"
            );
        } else {
            throw std::runtime_error(
                filename + " has an unknown record type at line " +
                std::to_string(rowLine) + "."
            );
        }
    }
    return hasExampleData;
}

struct VenueRoute {
    std::string nodeId;
    std::string note;
};

std::map<std::string, VenueRoute> loadVenueRoutes(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) throw std::runtime_error("Could not open " + filename + ".");
    std::string line;
    if (!std::getline(file, line)) {
        throw std::runtime_error(filename + " is empty.");
    }
    const std::vector<std::string> headers = parseCsvRow(line);
    const std::size_t venueColumn = findColumn(headers, "venue", filename);
    const std::size_t nodeColumn = findColumn(headers, "node_id", filename);
    const std::size_t noteColumn = findColumn(headers, "route_note", filename);

    std::map<std::string, VenueRoute> routes;
    std::size_t lineNumber = 1;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (trim(line).empty()) continue;
        const std::vector<std::string> fields = parseCsvRow(line);
        const std::string& venue = fieldAt(fields, venueColumn, filename, lineNumber);
        const std::string& nodeId = fieldAt(fields, nodeColumn, filename, lineNumber);
        const std::string& note = fieldAt(fields, noteColumn, filename, lineNumber);
        if (venue.empty() || nodeId.empty() || note.empty() ||
            !routes.emplace(venue, VenueRoute{nodeId, note}).second) {
            throw std::runtime_error(
                filename + " has an invalid or duplicate venue mapping at line " +
                std::to_string(lineNumber) + "."
            );
        }
    }
    if (file.bad()) throw std::runtime_error("Failed while reading " + filename + ".");
    return routes;
}

void printRoute(const CampusGraph& campus, const Route& route) {
    std::cout << std::fixed << std::setprecision(1)
              << "Shortest walking route (" << route.distance << " m, "
              << static_cast<int>(std::ceil(route.distance / 75.0)) << " min):\n";
    for (std::size_t i = 0; i < route.nodeIndexes.size(); ++i) {
        const CampusNode& node = campus.getNode(route.nodeIndexes[i]);
        if (i != 0) std::cout << " -> ";
        std::cout << node.name;
    }
    std::cout << '\n';
}

int routeToScheduledClass(
    CampusGraph& campus,
    const std::string& sectionName,
    int semester,
    int rollNumber,
    int g1Cutoff,
    const std::string& startNodeId,
    const std::string& day,
    int hour,
    int minute,
    const std::string& date
) {
    if (sectionName.empty() || semester < 1 || rollNumber < 1 ||
        g1Cutoff < 1 || !parseDate(date)) {
        throw std::runtime_error("Section, date, and positive student details are required.");
    }
    Section section(
        sectionName, "CLI", Venue("TBD"),
        "Not configured", "Not configured", g1Cutoff
    );
    Admin admin("Timetable routing CLI", "cli-admin");
    if (!admin.createTimetableForSection(section, semester)) {
        throw std::runtime_error("Could not initialize the requested timetable.");
    }
    const bool hasExampleData = loadTimetable(
        "data/timetable_examples.csv", sectionName, semester, section, admin
    );
    if (hasExampleData) {
        std::cout << "NOTICE: this timetable contains explicitly marked example data, "
                     "not an authoritative schedule.\n";
    }

    const Student student(
        "CLI student", std::to_string(rollNumber), &section, rollNumber
    );
    const CurrentClassInfo currentClass = LiveClassResolver::resolve(
        section, semester, day, hour, minute, date, student.getGroup()
    );
    if (!currentClass.found) {
        std::cout << "No class is scheduled for roll number " << rollNumber
                  << " at " << day << ' ' << std::setfill('0') << std::setw(2)
                  << hour << ':' << std::setw(2) << minute << ".\n";
        return 0;
    }
    if (currentClass.cancelled) {
        std::cout << currentClass.subjectName << " is cancelled for "
                  << date << "; no route is needed.\n";
        return 0;
    }

    const std::map<std::string, VenueRoute> venueRoutes =
        loadVenueRoutes("data/timetable_venue_nodes.csv");
    const auto destination = venueRoutes.find(currentClass.venueName);
    if (destination == venueRoutes.end()) {
        throw std::runtime_error(
            "No campus-map entrance is configured for timetable venue '" +
            currentClass.venueName + "' in data/timetable_venue_nodes.csv."
        );
    }

    Route route;
    if (!campus.shortestRoute(startNodeId, destination->second.nodeId, route)) {
        std::cout << "No walking route is available from " << startNodeId
                  << " to " << currentClass.venueName << ".\n";
        return 1;
    }

    std::cout << currentClass.subjectName << " with " << currentClass.facultyName
              << " is scheduled in " << currentClass.venueName << ".\n"
              << "Walking destination: "
              << campus.getNode(
                     route.nodeIndexes.back()
                 ).name
              << " (" << destination->second.note << ")\n";
    printRoute(campus, route);
    return 0;
}

bool readLine(const std::string& prompt, std::string& value) {
    std::cout << prompt;
    if (!std::getline(std::cin, value)) return false;
    value = trim(value);
    return true;
}

bool readRequired(const std::string& prompt, std::string& value) {
    for (;;) {
        if (!readLine(prompt, value)) return false;
        if (!value.empty()) return true;
        std::cout << "Please enter a value.\n";
    }
}

bool readPositiveInteger(const std::string& prompt, int& value) {
    for (;;) {
        std::string input;
        if (!readRequired(prompt, input)) return false;
        if (parseInteger(input, value) && value > 0) return true;
        std::cout << "Enter a positive whole number.\n";
    }
}

bool readDay(const std::string& prompt, std::string& day) {
    for (;;) {
        std::string input;
        if (!readRequired(prompt, input)) return false;
        day = normalizeDay(input);
        if (!day.empty()) return true;
        std::cout << "Enter MON, TUE, WED, THU, FRI, SAT, or SUN.\n";
    }
}

bool readTime(const std::string& prompt, int& hour, int& minute) {
    for (;;) {
        std::string input;
        if (!readRequired(prompt, input)) return false;
        if (parseTime(input, hour, minute)) return true;
        std::cout << "Enter time in 24-hour HH:MM format.\n";
    }
}

bool readDate(const std::string& prompt, std::string& date) {
    for (;;) {
        if (!readRequired(prompt, date)) return false;
        if (parseDate(date)) return true;
        std::cout << "Enter a valid date in YYYY-MM-DD format.\n";
    }
}

bool readStudentAndStart(
    std::string& section,
    int& semester,
    int& rollNumber,
    int& g1Cutoff,
    std::string& startNode
) {
    std::cout << "\nEnter your timetable and starting-location details.\n"
              << "Use option 5 in the main menu to find a location ID.\n";
    return readRequired("Section: ", section) &&
           readPositiveInteger("Semester: ", semester) &&
           readPositiveInteger("Roll number: ", rollNumber) &&
           readPositiveInteger("Last roll number in G1: ", g1Cutoff) &&
           readRequired("Starting campus location ID: ", startNode);
}

bool promptWithDefault(
    const std::string& prompt,
    const std::string& defaultValue,
    std::string& value
) {
    if (!readLine(prompt, value)) return false;
    if (value.empty()) value = defaultValue;
    return true;
}

bool promptPositiveIntWithDefault(
    const std::string& prompt,
    int defaultValue,
    int& value
) {
    for (;;) {
        std::string input;
        if (!readLine(prompt, input)) return false;
        if (input.empty()) {
            value = defaultValue;
            return true;
        }
        if (parseInteger(input, value) && value > 0) return true;
        std::cout << "Enter a positive whole number.\n";
    }
}

void printTimetable(const Section& section, int semester, bool isExample) {
    const Timetable* timetable = section.getTimetable(semester);
    if (timetable == nullptr) {
        throw std::runtime_error("The requested timetable was not initialized.");
    }
    if (isExample) {
        std::cout << "NOTICE: entries marked below are example data, "
                     "not an authoritative schedule.\n";
    }

    static const char* DAYS[] = {
        "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"
    };
    bool found = false;
    for (const char* day : DAYS) {
        std::vector<ScheduleEntry> entries = timetable->getEntriesForDay(day);
        std::sort(entries.begin(), entries.end(),
                  [](const ScheduleEntry& left, const ScheduleEntry& right) {
                      const TimeSlot leftSlot = left.getTimeSlot();
                      const TimeSlot rightSlot = right.getTimeSlot();
                      return leftSlot.getStartHour() * 60 + leftSlot.getStartMinute() <
                             rightSlot.getStartHour() * 60 + rightSlot.getStartMinute();
                  });
        if (entries.empty()) continue;
        found = true;
        std::cout << '\n' << day << ":\n";
        for (const ScheduleEntry& entry : entries) {
            const TimeSlot slot = entry.getTimeSlot();
            const std::shared_ptr<Subject> subject = entry.getSubject();
            std::cout << "  " << std::setfill('0') << std::setw(2)
                      << slot.getStartHour() << ':' << std::setw(2)
                      << slot.getStartMinute() << '-'
                      << std::setw(2) << slot.getEndHour() << ':'
                      << std::setw(2) << slot.getEndMinute()
                      << "  " << subject->getCourseCode()
                      << "  " << subject->getSubjectName()
                      << "  [" << subject->getFacultyName() << "]"
                      << "  @ " << entry.getVenue().getName()
                      << "  [" << groupScopeName(entry.getGroupScope()) << "]";
            if (isExample) std::cout << "  [EXAMPLE]";
            std::cout << '\n';
        }
    }
    if (!found) std::cout << "No classes are scheduled for that section/semester.\n";
}

void printMainMenu() {
    std::cout << "\n========================================\n"
              << "  STED Campus Timetable & Route Planner\n"
              << "========================================\n"
              << "1. Find my class and walking route now\n"
              << "2. Find my class and walking route at a chosen time\n"
              << "3. View a section's weekly timetable\n"
              << "4. Find a walking route between campus locations\n"
              << "5. List campus locations and IDs\n"
              << "0. Exit\n"
              << "Choose an option: ";
}

int runMenu(CampusGraph& campus) {
    for (;;) {
        printMainMenu();
        std::string choice;
        if (!std::getline(std::cin, choice)) {
            std::cout << "\nGoodbye.\n";
            return 0;
        }
        choice = trim(choice);
        try {
            if (choice == "0") {
                std::cout << "Goodbye.\n";
                return 0;
            }
            if (choice == "1" || choice == "2") {
                std::string section;
                std::string startNode;
                int semester = 0;
                int rollNumber = 0;
                int g1Cutoff = 0;
                if (!readStudentAndStart(
                        section, semester, rollNumber, g1Cutoff, startNode
                    )) {
                    std::cout << "\nGoodbye.\n";
                    return 0;
                }

                SystemClock clock;
                std::string day = clock.getCurrentDay();
                std::string date = clock.getCurrentDate();
                int hour = clock.getCurrentHour();
                int minute = clock.getCurrentMinute();
                if (choice == "2" &&
                    (!readDay("Day: ", day) ||
                     !readTime("Time (HH:MM): ", hour, minute) ||
                     !readDate("Date (YYYY-MM-DD): ", date))) {
                    std::cout << "\nGoodbye.\n";
                    return 0;
                }
                routeToScheduledClass(
                    campus, section, semester, rollNumber, g1Cutoff,
                    startNode, day, hour, minute, date
                );
            } else if (choice == "3") {
                std::string sectionName;
                int semester = 0;
                if (!promptWithDefault("Section [DEMO]: ", "DEMO", sectionName) ||
                    !promptPositiveIntWithDefault("Semester [1]: ", 1, semester)) {
                    std::cout << "\nGoodbye.\n";
                    return 0;
                }
                Section section(
                    sectionName, "CLI", Venue("TBD"),
                    "Not configured", "Not configured", 20
                );
                Admin admin("Timetable viewer", "cli-viewer");
                if (!admin.createTimetableForSection(section, semester)) {
                    throw std::runtime_error("Could not initialize the timetable.");
                }
                const bool isExample = loadTimetable(
                    "data/timetable_examples.csv", sectionName,
                    semester, section, admin
                );
                std::cout << "\nWeekly timetable for " << sectionName
                          << ", semester " << semester << ":\n";
                printTimetable(section, semester, isExample);
            } else if (choice == "4") {
                std::string start;
                std::string destination;
                if (!readRequired("Starting location ID: ", start) ||
                    !readRequired("Destination location ID: ", destination)) {
                    std::cout << "\nGoodbye.\n";
                    return 0;
                }
                Route route;
                if (!campus.shortestRoute(start, destination, route)) {
                    std::cout << "No walking route is available between those locations.\n";
                } else {
                    printRoute(campus, route);
                }
            } else if (choice == "5") {
                std::cout << "\nCampus locations (use an ID as the start location):\n";
                campus.listPlaces();
            } else {
                std::cout << "Invalid choice. Select 0, 1, 2, 3, 4, or 5.\n";
            }
        } catch (const std::exception& error) {
            if (!std::cin) {
                std::cout << "\nGoodbye.\n";
                return 0;
            }
            std::cerr << "Could not complete that option: " << error.what() << '\n';
        }
    }
}

int runTimetableRoute(
    int argc,
    char* argv[],
    CampusGraph& campus
) {
    if (argc != 8 && argc != 10) {
        throw std::runtime_error(
            "Usage: sted_graph.exe --timetable-route <section> <semester> "
            "<roll-number> <g1-cutoff> <start-node-id> --now\n"
            "   or: sted_graph.exe --timetable-route <section> <semester> "
            "<roll-number> <g1-cutoff> <start-node-id> <DAY> <HH:MM> <YYYY-MM-DD>"
        );
    }
    int semester = 0;
    int rollNumber = 0;
    int g1Cutoff = 0;
    if (!parseInteger(argv[3], semester) || semester < 1 ||
        !parseInteger(argv[4], rollNumber) || rollNumber < 1 ||
        !parseInteger(argv[5], g1Cutoff) || g1Cutoff < 1) {
        throw std::runtime_error(
            "Semester, roll number, and G1 cutoff must be positive integers."
        );
    }

    SystemClock clock;
    std::string day = clock.getCurrentDay();
    std::string date = clock.getCurrentDate();
    int hour = clock.getCurrentHour();
    int minute = clock.getCurrentMinute();
    if (argc == 8) {
        if (std::string(argv[7]) != "--now") {
            throw std::runtime_error("Use --now or provide DAY HH:MM YYYY-MM-DD.");
        }
    } else {
        day = normalizeDay(argv[7]);
        date = argv[9];
        if (day.empty() || !parseTime(argv[8], hour, minute) || !parseDate(date)) {
            throw std::runtime_error(
                "Invalid day, time, or date. Use DAY, HH:MM, and YYYY-MM-DD."
            );
        }
    }
    return routeToScheduledClass(
        campus, argv[2], semester, rollNumber, g1Cutoff, argv[6],
        day, hour, minute, date
    );
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        CampusGraph campus;
        campus.loadNodes("data/campus_node_coordinates.csv");
        campus.loadEdges("data/campus_edges.csv");
        std::cout << "Loaded " << campus.nodeCount()
                  << " campus locations from CSV data.\n";

        if (argc == 1) {
            return runMenu(campus);
        }
        if (argc == 2 && std::string(argv[1]) == "--places") {
            std::cout << "Campus locations (use an ID as the start location):\n";
            campus.listPlaces();
            return 0;
        }
        if (argc == 2 && std::string(argv[1]) == "--help") {
            std::cout << "Run sted_graph.exe with no arguments for the interactive menu.\n"
                      << "Shortcuts:\n"
                      << "  sted_graph.exe --route <start-id> <destination-id>\n"
                      << "  sted_graph.exe --timetable-route <section> <semester> "
                         "<roll-number> <g1-cutoff> <start-id> --now\n"
                      << "  sted_graph.exe --timetable-route <section> <semester> "
                         "<roll-number> <g1-cutoff> <start-id> "
                         "<DAY> <HH:MM> <YYYY-MM-DD>\n"
                      << "  sted_graph.exe --places\n";
            return 0;
        }
        if (std::string(argv[1]) == "--timetable-route") {
            return runTimetableRoute(argc, argv, campus);
        }
        if (argc != 4 || std::string(argv[1]) != "--route") {
            std::cerr << "Usage: sted_graph.exe --route <start-id> <destination-id>\n"
                      << "       sted_graph.exe --timetable-route <section> <semester> "
                         "<roll-number> <g1-cutoff> <start-id> --now\n"
                      << "       sted_graph.exe --places\n";
            return 2;
        }

        Route route;
        if (!campus.shortestRoute(argv[2], argv[3], route)) {
            std::cout << "No route is available between those locations.\n";
            return 1;
        }
        printRoute(campus, route);
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
