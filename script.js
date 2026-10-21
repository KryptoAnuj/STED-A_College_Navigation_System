let scrollFrame = null;
let restoreScrollBehavior = null;
let completeActiveScroll = null;
let sectionHighlightTimer = null;
let highlightedSection = null;
let returnHomeTimer;

function resetHomeTimer() {
  clearTimeout(returnHomeTimer);

  returnHomeTimer = setTimeout(() => {
    campusMap.setView(homeLocation, homeZoom, {
      animate: true,
    });
  }, 10000);
}

campusMap.on("dragstart zoomstart", resetHomeTimer);

resetHomeTimer();

document.addEventListener("copy", (e) => e.preventDefault());
document.addEventListener("cut", (e) => e.preventDefault());
document.addEventListener("paste", (e) => e.preventDefault());
document.addEventListener("contextmenu", (e) => e.preventDefault());

function highlightSection(target) {
  clearTimeout(sectionHighlightTimer);
  highlightedSection?.classList.remove("section-blink");
  target.classList.remove("section-blink");
  void target.offsetWidth;
  target.classList.add("section-blink");
  highlightedSection = target;

  sectionHighlightTimer = setTimeout(() => {
    target.classList.remove("section-blink");
    highlightedSection = null;
  }, 800);
}

function scrollToSection(hash) {
  const target = document.getElementById(decodeURIComponent(hash.slice(1)));

  if (!target) {
    return;
  }

  target.focus({ preventScroll: true });
  highlightSection(target);

  if (scrollFrame !== null) {
    cancelAnimationFrame(scrollFrame);
    scrollFrame = null;
    completeActiveScroll = null;
    restoreScrollBehavior?.();
  }

  const targetPosition = Math.max(
    0,
    Math.min(
      target.getBoundingClientRect().top +
        window.scrollY -
        parseFloat(getComputedStyle(target).scrollMarginTop || "0"),
      document.documentElement.scrollHeight - window.innerHeight
    )
  );
  const prefersReducedMotion = window.matchMedia(
    "(prefers-reduced-motion: reduce)"
  ).matches;

  if (prefersReducedMotion) {
    window.scrollTo({ top: targetPosition, behavior: "instant" });
    return;
  }

  const startPosition = window.scrollY;
  const startTime = performance.now();
  const duration = 700;
  const root = document.documentElement;
  const previousScrollBehavior = root.style.scrollBehavior;
  root.style.scrollBehavior = "auto";
  restoreScrollBehavior = () => {
    root.style.scrollBehavior = previousScrollBehavior;
    restoreScrollBehavior = null;
  };
  completeActiveScroll = () => {
    window.scrollTo(0, targetPosition);
    restoreScrollBehavior?.();
    completeActiveScroll = null;
    scrollFrame = null;
  };

  function animateScroll(currentTime) {
    const progress = Math.min((currentTime - startTime) / duration, 1);
    const easedProgress =
      progress < 0.5
        ? 4 * progress ** 3
        : 1 - (-2 * progress + 2) ** 3 / 2;

    window.scrollTo(0, startPosition + (targetPosition - startPosition) * easedProgress);

    if (progress < 1) {
      scrollFrame = requestAnimationFrame(animateScroll);
      return;
    }

    completeActiveScroll();
  }

  scrollFrame = requestAnimationFrame(animateScroll);
}

function updateClock() {
  const now = new Date();

  document.getElementById("liveClock").textContent =
    now.toLocaleTimeString("en-IN", {
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
      hour12: true,
      timeZone: "Asia/Kolkata"
    });

  document.getElementById("liveDate").textContent =
    now.toLocaleDateString("en-IN", {
      weekday: "short",
      day: "2-digit",
      month: "short",
      year: "numeric",
      timeZone: "Asia/Kolkata"
    });
}

updateClock();
setInterval(updateClock, 1000);

document.addEventListener("visibilitychange", () => {
  if (document.hidden && scrollFrame !== null) {
    cancelAnimationFrame(scrollFrame);
    completeActiveScroll?.();
  }
});

document.querySelectorAll('a[href^="#"]').forEach((link) => {
  link.addEventListener("click", (event) => {
    if (
      event.defaultPrevented ||
      event.button !== 0 ||
      event.metaKey ||
      event.ctrlKey ||
      event.shiftKey ||
      event.altKey
    ) {
      return;
    }

    const target = document.getElementById(decodeURIComponent(link.hash.slice(1)));

    if (!target) {
      return;
    }

    event.preventDefault();

    if (window.location.hash !== link.hash) {
      window.history.pushState(null, "", link.hash);
    }

    scrollToSection(link.hash);
  });
});

window.addEventListener("popstate", () => {
  if (window.location.hash) {
    scrollToSection(window.location.hash);
  }
});

const campusNodes = {
  "Reception": { x: 16, y: 56 },
  "CR-15": { x: 30, y: 46 },
  "Staircase": { x: 42, y: 43 },
  "CR-16": { x: 52, y: 48 },
  "LT-12": { x: 60, y: 27 },
  "LT-13": { x: 80, y: 27 },
  "Front Corridor": { x: 64, y: 70 },
  "Project Lab": { x: 82, y: 64 },
  "Open Audi": { x: 88, y: 40 },
  "Basketball Court": { x: 88, y: 77 }
};

const campusPaths = [
  ["Reception", "CR-15", 10, true],
  ["Reception", "Front Corridor", 150, true],
  ["CR-15", "Staircase", 15, false],
  ["Staircase", "CR-16", 10, false],
  ["CR-16", "LT-12", 12, true],
  ["LT-12", "LT-13", 10, true],
  ["LT-12", "Front Corridor", 80, true],
  ["LT-13", "Front Corridor", 70, true],
  ["Front Corridor", "Project Lab", 20, true],
  ["Front Corridor", "Open Audi", 100, true],
  ["Front Corridor", "Basketball Court", 200, true]
];

const demoSchedule = [
  { startTime: "08:30", title: "Data Structures", location: "CR-15" },
  { startTime: "10:00", title: "Operating Systems", location: "LT-12" },
  { startTime: "12:30", title: "Seminar", location: "Conference Hall" },
  { startTime: "14:00", title: "Project Lab", location: "Project Lab" }
];

const startSelect = document.getElementById("startSelect");
const destinationSelect = document.getElementById("destinationSelect");
const routeForm = document.getElementById("routeForm");
const routeTitle = document.getElementById("routeTitle");
const routeDuration = document.getElementById("routeDuration");
const routeDistance = document.getElementById("routeDistance");
const routeStatus = document.getElementById("routeStatus");
const nextStop = document.getElementById("nextStop");
const pathList = document.getElementById("pathList");
const connections = document.getElementById("connections");
const scheduleList = document.getElementById("scheduleList");
const accessibleToggle = document.getElementById("accessibleToggle");
const travelMode = document.getElementById("travelMode");
const directorySearch = document.getElementById("directorySearch");
const floorFilter = document.getElementById("floorFilter");
const directoryList = document.getElementById("directoryList");
const directoryCount = document.getElementById("directoryCount");
const campusCsvFiles = [
  "data/BTECH.csv"
];

async function loadCampusLocations() {
  const datasets = await Promise.all(
    campusCsvFiles.map(async (file) => {
      const response = await fetch(file);

      if (!response.ok) {
        throw new Error(`Failed to load ${file}: ${response.status}`);
      }

      const csvText = await response.text();

      const result = Papa.parse(csvText, {
        header: true,
        skipEmptyLines: true,
        transformHeader: header => header.trim()
      });

      if (result.errors.length > 0) {
        console.warn(`CSV parsing warnings in ${file}:`, result.errors);
      }

      return result.data.map(row => ({
        building: (row.building || "").trim(),
        floor: (row.floor || "").trim(),
        room: (row.room || "").trim(),
        description: (row.description || "").trim(),
        side: (row.side || "").trim()
      }));
    })
  );

  return datasets.flat();
}

function populateRouteLocations() {
  Object.keys(campusNodes).forEach((name) => {
    [startSelect, destinationSelect].forEach((select) => {
      const option = document.createElement("option");
      option.value = name;
      option.textContent = name;
      select.appendChild(option);
    });
  });
  startSelect.value = "Reception";
  destinationSelect.value = "Project Lab";
}

function findRoute(start, destination, accessible) {
  const names = Object.keys(campusNodes);
  const distances = Object.fromEntries(names.map((name) => [name, Infinity]));
  const previous = new Map();
  const remaining = new Set(names);
  distances[start] = 0;

  while (remaining.size) {
    let current;
    for (const name of remaining) {
      if (current === undefined || distances[name] < distances[current]) {
        current = name;
      }
    }
    remaining.delete(current);
    if (current === destination || distances[current] === Infinity) break;

    campusPaths.forEach(([from, to, distance, isAccessible]) => {
      if (accessible && !isAccessible) return;
      const neighbor = from === current ? to : to === current ? from : null;
      if (!neighbor || !remaining.has(neighbor)) return;
      const candidate = distances[current] + distance;
      if (candidate < distances[neighbor]) {
        distances[neighbor] = candidate;
        previous.set(neighbor, current);
      }
    });
  }

  if (start !== destination && !previous.has(destination)) return null;

  const path = [destination];
  while (path[0] !== start) path.unshift(previous.get(path[0]));
  const directions = path.slice(1).map((to, index) => {
    const from = path[index];
    const edge = campusPaths.find(([a, b]) =>
      (a === from && b === to) || (a === to && b === from)
    );
    const text = index === 0
      ? `Head toward ${to}`
      : index === path.length - 2
        ? `Arrive at ${to}`
        : `Continue to ${to}`;
    return { text, from, to, distance: edge[2] };
  });

  return { path, distance: distances[destination], directions };
}

function renderPath(route) {
  const activeNodes = new Set(route.path);
  document.querySelectorAll(".map-node").forEach((node) => {
    node.classList.toggle("active", activeNodes.has(node.dataset.node));
  });

  const line = document.createElementNS("http://www.w3.org/2000/svg", "path");
  line.setAttribute("d", route.path.map((name, index) => {
    const point = campusNodes[name];
    return `${index === 0 ? "M" : "L"} ${point.x} ${point.y}`;
  }).join(" "));
  connections.replaceChildren(line);
  pathList.replaceChildren();

  if (!route.directions.length) {
    const item = document.createElement("li");
    const text = document.createElement("strong");
    text.textContent = "You are already at your destination.";
    item.appendChild(text);
    pathList.appendChild(item);
    return;
  }

  route.directions.forEach((direction, index) => {
    const item = document.createElement("li");
    const details = document.createElement("div");
    const text = document.createElement("strong");
    const step = document.createElement("small");
    const distance = document.createElement("span");
    text.textContent = direction.text;
    step.textContent = index === 0 ? "Starting point" : `Step ${index + 1}`;
    distance.className = "distance-tag";
    distance.textContent = `${direction.distance} m`;
    details.append(text, step);
    item.append(details, distance);
    pathList.appendChild(item);
  });
}

function updateRoute() {
  const start = startSelect.value;
  const destination = destinationSelect.value;
  const accessible = accessibleToggle.checked || travelMode.value !== "Walking";
  routeTitle.textContent = `${start} → ${destination}`;

  const route = findRoute(start, destination, accessible);
  if (!route) {
    routeStatus.textContent = "No route available";
    routeDuration.textContent = "—";
    routeDistance.textContent = "—";
    nextStop.textContent = "Unavailable";
    pathList.replaceChildren();
    connections.replaceChildren();
    document.querySelectorAll(".map-node").forEach((node) => node.classList.remove("active"));
    return;
  }

  const duration = route.distance === 0
    ? 0
    : Math.max(3, Math.round(route.distance / 25));
  routeDuration.textContent = `${duration} min`;
  routeDistance.textContent = `${route.distance} m`;
  routeStatus.textContent = duration <= 8 ? "On time" : "Slight delay";
  nextStop.textContent = route.path[1] || destination;
  document.getElementById("routeClassroom").textContent = destination;
  renderPath(route);
}

function renderSchedule() {
  scheduleList.replaceChildren();
  demoSchedule.forEach((entry) => {
    const item = document.createElement("li");
    const time = document.createElement("div");
    const details = document.createElement("div");
    const title = document.createElement("strong");
    const location = document.createElement("span");
    time.className = "time";
    time.textContent = entry.startTime;
    details.className = "schedule-text";
    title.textContent = entry.title;
    location.textContent = entry.location;
    details.append(title, location);
    item.append(time, details);
    scheduleList.appendChild(item);
  });
}

async function initializeCampusApp() {
  const campusLocations = await loadCampusLocations();

  console.log("Loaded campus locations:", campusLocations.length);

  // Put the rest of your existing script.js code here.
  // Search, filters, and other code that uses campusLocations
  // must run after the CSV files have loaded.
}

initializeCampusApp().catch(error => {
  console.error("Could not initialize campus data:", error);
});

function renderBuildingDirectory() {
  const query = directorySearch.value.trim().toLocaleLowerCase();
  const selectedFloor = floorFilter.value;
  const matchingLocations = campusLocations.filter((location) => {
    const matchesFloor = !selectedFloor || location.floor === selectedFloor;
    const searchableText = [
      location.building,
      location.floor,
      location.room,
      location.description,
      location.side
    ].join(" ").toLocaleLowerCase();
    return matchesFloor && searchableText.includes(query);
  });

  directoryCount.textContent =
    `${matchingLocations.length} of ${campusLocations.length} locations`;
  directoryList.replaceChildren();

  if (!matchingLocations.length) {
    const emptyState = document.createElement("li");
    emptyState.textContent = campusLocations.length
      ? "No rooms or facilities match this filter."
      : "Campus directory data could not be loaded.";
    directoryList.appendChild(emptyState);
    return;
  }

  matchingLocations.forEach((location) => {
    const item = document.createElement("li");
    const name = document.createElement("strong");
    const details = document.createElement("span");
    const locationName = location.room || location.description;
    name.textContent = location.room || location.description;
    details.textContent = [
      location.floor,
      location.side,
      location.room && location.description ? location.description : ""
    ].filter(Boolean).join(" · ");
    item.append(name, details);
    if (Object.hasOwn(campusNodes, locationName)) {
      const routeButton = document.createElement("button");
      routeButton.className = "ghost-button small directory-route-button";
      routeButton.type = "button";
      routeButton.textContent = "Plan route here";
      routeButton.addEventListener("click", () => {
        destinationSelect.value = locationName;
        updateRoute();
        document.getElementById("routes").scrollIntoView({ behavior: "smooth" });
      });
      item.appendChild(routeButton);
    }
    directoryList.appendChild(item);
  });
}

function initializeDashboard() {
  document.getElementById("nextLectureLabel").textContent =
    "Interactive route preview · sample timetable";
  document.querySelector(".status-row").lastChild.textContent =
    "BTech campus directory loaded";
  document.getElementById("nextLectureRoom").textContent = "Project Lab";
  document.getElementById("nextLectureEta").textContent = "7 min";
  document.getElementById("nextLectureDistance").textContent = "170 m";

  populateRouteLocations();
  renderSchedule();

  const floors = [...new Set(campusLocations.map((location) => location.floor))];
  floors.forEach((floor) => {
    const option = document.createElement("option");
    option.value = floor;
    option.textContent = floor;
    floorFilter.appendChild(option);
  });
  renderBuildingDirectory();
  updateRoute();
}

routeForm.addEventListener("submit", (event) => {
  event.preventDefault();
  updateRoute();
  document.getElementById("campus-map").scrollIntoView({ behavior: "smooth" });
});
startSelect.addEventListener("change", updateRoute);
destinationSelect.addEventListener("change", updateRoute);
accessibleToggle.addEventListener("change", updateRoute);
travelMode.addEventListener("change", updateRoute);
directorySearch.addEventListener("input", renderBuildingDirectory);
floorFilter.addEventListener("change", renderBuildingDirectory);

document.getElementById("showRouteButton").addEventListener("click", () => {
  document.getElementById("routes").scrollIntoView({ behavior: "smooth" });
});

initializeDashboard();
