<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
</head>
<body>

<!-- 1. Project's Title -->
<div class="center">
  <h1>🧭 Sted — A College Navigation System</h1>
  <p>Sted has two independent implementations: a static website for presenting campus navigation and the screenshot-based timetable, and a standalone C++ terminal program for academic demonstrations of graphs, routing, and timetable resolution.</p>
</div>

<!-- 2. Project Description -->
<h2 id="description">📌 Description</h2>
<p>
Students and faculty often struggle to reach their next lecture on time when classrooms are located across
different campus blocks or are reassigned at short notice. Existing timetable systems tell users what class
and room they're scheduled for, but they don't help with actually navigating there. <strong>Sted</strong> closes
that gap by combining timetable management with shortest-path campus navigation — given a user's current
location, it identifies their next lecture, computes the shortest route to that classroom, and automatically recalculates and notifies affected users whenever a classroom is changed.

<h2>Run the terminal program</h2>
<p>Build from the repository root with CMake and a C++ compiler. CMake compiles all C++ modules into one executable and embeds the graph and timetable CSV data, so the built executable runs without the repository or a separate data folder.</p>
<pre><code>cmake -S . -B build/cpp
cmake --build build/cpp --config Release</code></pre>
<p>On Windows with a multi-configuration generator, run <code>build\cpp\Release\sted_graph.exe</code>; with a single-configuration generator, run <code>build\cpp\sted_graph.exe</code>.</p>

<h2>Preview the website</h2>
<p>The website loads its own static files and building directories from the repository. From the repository root, run:</p>
<pre><code>python -m http.server 8000 --bind 127.0.0.1</code></pre>
<p>Open <a href="http://127.0.0.1:8000">http://127.0.0.1:8000</a>. The website and terminal program remain separate and do not call each other.</p>
</p>

<!-- 3. Credits -->
<h2 id="credits">🙏 Credits</h2>
<table>
  <tr>
    <th>Name</th>
    <th>Role</th>
    <th>GitHub</th>
  </tr>
  <tr>
    <td>Anuj Agarwal</td>
    <td>Leader</td>
    <td><a href="https://github.com/KryptoAnuj">@KryptoAnuj</a></td>
  </tr>
  <tr>
    <td>Atharv Kamboj</td>
    <td>Developer</td>
    <td><a href="https://github.com/atharv229">@atharv229</a></td>
  </tr>
  <tr>
    <td>Ayush Bhalla</td>
    <td>DB/Testing</td>
    <td><a href="https://github.com/Ayushbhalla">@Ayushbhalla</a></td>
  </tr>
  <tr>
    <td>Parth Raj Jaiswal</td>
    <td>Developer/Docs</td>
    <td><a href="https://github.com/ParthJaiswal-md">@ParthJaiswal-md</a></td>
  </tr>
</table>

<h2 id="credits">📑 References</h2>
<ul>
  <li><a href="https://ieeexplore.ieee.org/document/10001791">IEEE Xplore — related navigation systems research</a></li>
  <li><a href="https://blog.evelity.com/en/students-getting-lost-on-campus-2025-survey">Evelity Blog — Students Getting Lost on Campus (2025 Survey)</a></li>
  <li><a href="https://www.mazemap.com/post/hidden-drivers-of-student-retention">MazeMap — Hidden Drivers of Student Retention</a></li>
</ul>


</body>
</html>
