<div align="center">
🧭 Sted: A College Navigation System

Timetable management and shortest-path campus navigation in one system.

</div>

Sted has two independent implementations: a static website for presenting campus navigation and the screenshot-based timetable, and a standalone C++ terminal program for academic demonstrations of graphs, routing, and timetable resolution.

📑 Table of Contents
Description
Problem Statement
Objectives
Key Features
The Two Implementations
How It Works
Tech Stack
Project Structure
Getting Started
Usage
Future Scope
Credits
References
📌 Description

Students and faculty often struggle to reach their next lecture on time when classrooms are located across different campus blocks or are reassigned at short notice. Existing timetable systems tell users what class and room they are scheduled for, but they do not help with actually navigating there.

Sted closes that gap by combining timetable management with shortest-path campus navigation. Given a user's current location, Sted:

Identifies their next lecture from the timetable.
Computes the shortest route to that classroom.
Automatically recalculates the route and notifies affected users whenever a classroom is changed.
❗ Problem Statement

Campus life depends on being in the right room at the right time, but three problems keep getting in the way:

Spread-out campuses: classrooms sit in different blocks, so a wrong turn can mean a late arrival.
Last-minute room changes: when a classroom is reassigned at short notice, students and faculty may not find out in time.
A gap between timetable and navigation: a timetable says where to go but never how to get there.
🎯 Objectives
Model the campus as a graph, with locations as nodes and walkable paths as weighted edges.
Compute the shortest path between any two locations on campus.
Resolve a user's next lecture from the timetable data based on their current location.
Handle classroom changes by recalculating routes and notifying the affected users.
Present the system in two forms: a website for visual demonstration and a terminal program for demonstrating the underlying data structures and algorithms.
✨ Key Features
Feature	What it does
🗺️ Shortest-path navigation	Finds the quickest route between a user's current location and their destination classroom.
📅 Next-lecture detection	Reads the timetable and works out which lecture comes next for the user.
🔄 Automatic recalculation	When a classroom is changed, the route is recomputed without manual effort.
🔔 Change notifications	Users affected by a classroom change are notified.
🖼️ Screenshot-based timetable	The website presents the timetable as a screenshot-based view.
🏢 Building directories	The website loads building directories from the repository.
💻 Standalone terminal program	A C++ program that demonstrates graphs, routing, and timetable resolution.
📦 Self-contained executable	The graph and timetable CSV data are embedded in the executable, so it runs without the repository or a separate data folder.
🧩 The Two Implementations

Sted is built as two separate parts. They do not call each other, and each one can be used on its own.

🌐 1. Static Website
Presents the campus navigation and the screenshot-based timetable.
Loads its own static files and building directories from the repository.
Needs no backend. Any simple static file server can host it.
💻 2. C++ Terminal Program (sted_graph)
A standalone program for academic demonstrations of:
Graphs: representing the campus as nodes and edges.
Routing: finding the shortest path between locations.
Timetable resolution: working out the next lecture and its classroom.
Built with CMake, which compiles all C++ modules into one executable.
The graph and timetable CSV data are embedded in the executable at build time.
⚙️ How It Works
User enters current location
Read timetable data
Identify next lecture and itsclassroom
Compute shortest route oncampus graph
Show route to the user
Classroom is changed
Recalculate route
Notify affected users

In plain English:

The campus is stored as a graph: each building or room is a point, and each walkable connection between them has a distance.
The timetable is stored as CSV data listing which lecture happens in which room and when.
When a user gives their location, the system looks up their next lecture and the room it is in.
It then finds the shortest path from the user's location to that room on the graph.
If a classroom changes, the affected lectures are updated, routes are recalculated, and the people affected are notified.
🛠️ Tech Stack
Part	Technologies
Terminal program	C++, CMake
Data	CSV files for the graph and the timetable (embedded into the executable at build time)
Website	Static HTML, CSS, and JavaScript
Local preview	Python's built-in http.server
📂 Project Structure

📝 Replace the entries below with the real file and folder names from your repository.

text
STED-A_College_Navigation_System/
├── CMakeLists.txt        # Build configuration for the C++ terminal program
├── cpp/ (or src/)        # C++ source modules (graph, routing, timetable)
├── data/                 # Graph and timetable CSV files (embedded at build time)
├── index.html            # Website entry point
├── css/ js/ assets/      # Website static files
├── buildings/            # Building directories used by the website
└── README.md
🚀 Getting Started
Prerequisites
For the terminal program: CMake and a C++ compiler (for example GCC, Clang, or MSVC).
For the website: Python 3 (only needed to run the local preview server) and any modern web browser.
Clone the repository
bash
git clone https://github.com/KryptoAnuj/STED-A_College_Navigation_System.git
cd STED-A_College_Navigation_System
💻 Run the terminal program

Build from the repository root with CMake and a C++ compiler. CMake compiles all C++ modules into one executable and embeds the graph and timetable CSV data, so the built executable runs without the repository or a separate data folder.

bash
cmake -S . -B build/cpp
cmake --build build/cpp --config Release

Then run the program:

Setup	Command
Windows, multi-configuration generator	build\cpp\Release\sted_graph.exe
Windows, single-configuration generator	build\cpp\sted_graph.exe
Linux / macOS	./build/cpp/sted_graph
🌐 Preview the website

The website loads its own static files and building directories from the repository. From the repository root, run:

bash
python -m http.server 8000 --bind 127.0.0.1

Open http://127.0.0.1:8000 in your browser.

The website and the terminal program remain separate and do not call each other.

📖 Usage

Website

Open the local preview link in your browser.
Browse the campus navigation and the screenshot-based timetable.
Use the building directories to find the location you need.

Terminal program

Build and run sted_graph as shown above.
Follow the on-screen prompts to explore the campus graph, check routes, and resolve timetable entries.

📝 Add a short example of an input and the output it gives (for example a start location, the next lecture found, and the route printed).

🔮 Future Scope
Connect the website and the terminal logic so one system powers both.
Add live notifications for classroom changes on mobile or by email.
Support indoor floor-wise navigation inside each building.
Add accessibility-friendly routes, such as paths avoiding stairs.
Import a student's own timetable automatically.
🙏 Credits
Name	Role	GitHub
Anuj Agarwal	Leader	@KryptoAnuj
Atharv Kamboj	Developer	@atharv229
Ayush Bhalla	DB/Testing	@Aayush-Bhalla
Parth Raj Jaiswal	Developer/Docs	@ParthJaiswal-md
📑 References
IEEE Xplore: related navigation systems research
Evelity Blog: Students Getting Lost on Campus (2025 Survey)
MazeMap: Hidden Drivers of Student Retention
