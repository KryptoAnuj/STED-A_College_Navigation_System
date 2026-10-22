cmake_minimum_required(VERSION 3.16)
project(STEDCollegeNavigation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sted_graph main.cpp)

add_library(sted_timetable
  TimeTable/Core.cpp
  TimeTable/LiveClock.cpp
  TimeTable/ScheduleEntry.cpp
  TimeTable/Section.cpp
  TimeTable/Timetable.cpp
  TimeTable/User.cpp
)
target_include_directories(sted_timetable PUBLIC TimeTable)
target_link_libraries(sted_graph PRIVATE sted_timetable)
