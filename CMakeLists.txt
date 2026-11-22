cmake_minimum_required(VERSION 3.16)
project(STEDCollegeNavigation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Embed the resources needed by the standalone terminal application.
set(EMBEDDED_CSV_FILES
  data/node_coordinates.csv
  data/campus_edges.csv
  data/timetable_examples.csv
  data/timetable_venue_nodes.csv
)
# Generate a lookup function so the executable does not depend on runtime CSV files.
set(EMBEDDED_CSV_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/embedded_csv_data.cpp")
file(WRITE "${EMBEDDED_CSV_SOURCE}"
  "#include \"EmbeddedData.h\"\n"
  "#include <stdexcept>\n"
  "#include <unordered_map>\n\n"
  "std::string embeddedCsv(const std::string& path) {\n"
  "  static const std::unordered_map<std::string, std::string> files = {\n"
)

foreach(csv_path IN LISTS EMBEDDED_CSV_FILES)
  set(csv_file "${CMAKE_CURRENT_SOURCE_DIR}/${csv_path}")
  if(NOT EXISTS "${csv_file}")
    message(FATAL_ERROR "Required C++ data file is missing: ${csv_path}")
  endif()
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${csv_file}")
  file(READ "${csv_file}" csv_contents)
  string(SHA256 csv_hash "${csv_contents}")
  string(SUBSTRING "${csv_hash}" 0 12 csv_delimiter_suffix)
  set(csv_delimiter "STED${csv_delimiter_suffix}")
  string(FIND "${csv_contents}" ")${csv_delimiter}\"" delimiter_collision)
  if(NOT delimiter_collision EQUAL -1)
    message(FATAL_ERROR "Could not safely embed ${csv_path}: raw-string delimiter collision.")
  endif()

  file(APPEND "${EMBEDDED_CSV_SOURCE}"
    "    {\"${csv_path}\", R\"${csv_delimiter}("
  )
  file(APPEND "${EMBEDDED_CSV_SOURCE}" "${csv_contents}")
  file(APPEND "${EMBEDDED_CSV_SOURCE}" ")${csv_delimiter}\"},\n")
endforeach()

file(APPEND "${EMBEDDED_CSV_SOURCE}"
  "  };\n"
  "  const auto found = files.find(path);\n"
  "  if (found == files.end()) {\n"
  "    throw std::runtime_error(\"Missing embedded CSV resource: \" + path);\n"
  "  }\n"
  "  return found->second;\n"
  "}\n"
)

add_executable(
  sted_graph
  main.cpp
  "${EMBEDDED_CSV_SOURCE}"
  TimeTable/Core.cpp
  TimeTable/LiveClock.cpp
  TimeTable/ScheduleEntry.cpp
  TimeTable/Section.cpp
  TimeTable/Timetable.cpp
  TimeTable/User.cpp
)
target_include_directories(sted_graph PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
