let scrollFrame = null;
let restoreScrollBehavior = null;
let completeActiveScroll = null;
let sectionHighlightTimer = null;
let highlightedSection = null;
let returnHomeTimer;
let mapManuallyExploring = false;
let activeRouteKey = "";
let activeRouteBounds = null;

// Only restore the campus view when no route or manual map exploration is active.
function resetHomeTimer() {
  clearTimeout(returnHomeTimer);
  returnHomeTimer = setTimeout(() => {
    if (
      campusBounds &&
      !activeRouteBounds &&
      !mapManuallyExploring &&
      !navigationMode &&
      !usingCurrentLocation
    ) {
      campusMap.fitBounds(campusBounds, {
        padding: [20, 20],
        maxZoom: homeZoom,
        animate: true
      });
    }
  }, 10000);
}

campusMap.on("dragstart zoomstart", resetHomeTimer);
const mapContainer = campusMap.getContainer();
["pointerdown", "touchstart", "wheel", "keydown"].forEach((eventName) => {
  mapContainer.addEventListener(eventName, () => {
    mapManuallyExploring = true;
  }, { passive: true });
});
resetHomeTimer();

document.addEventListener("copy", (event) => event.preventDefault());
document.addEventListener("cut", (event) => event.preventDefault());
document.addEventListener("paste", (event) => event.preventDefault());
document.addEventListener("contextmenu", (event) => event.preventDefault());

// Keep in-page navigation animated and visibly mark the selected section.
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
  if (!target) return;

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
    window.scrollTo(
      0,
      startPosition + (targetPosition - startPosition) * easedProgress
    );

    if (progress < 1) {
      scrollFrame = requestAnimationFrame(animateScroll);
      return;
    }
    completeActiveScroll();
  }

  scrollFrame = requestAnimationFrame(animateScroll);
}

// Update the live clock and date using the campus time zone.
function updateClock() {
  const now = new Date();
  document.getElementById("liveClock").textContent = now.toLocaleTimeString(
    "en-IN",
    {
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
      hour12: true,
      timeZone: "Asia/Kolkata"
    }
  ).toUpperCase();
  document.getElementById("liveDate").textContent = now.toLocaleDateString(
    "en-IN",
    {
      weekday: "short",
      day: "2-digit",
      month: "short",
      year: "numeric",
      timeZone: "Asia/Kolkata"
    }
  );
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
    if (!target) return;

    event.preventDefault();
    if (window.location.hash !== link.hash) {
      window.history.pushState(null, "", link.hash);
    }
    scrollToSection(link.hash);
  });
});

window.addEventListener("popstate", () => {
  if (window.location.hash) scrollToSection(window.location.hash);
});

const startSelect = document.getElementById("startSelect");
const destinationSelect = document.getElementById("destinationSelect");
const routeForm = document.getElementById("routeForm");
const routeTitle = document.getElementById("routeTitle");
const routeDuration = document.getElementById("routeDuration");
const routeDistance = document.getElementById("routeDistance");
const directionsDistance = document.getElementById("directionsDistance");
const directionsDuration = document.getElementById("directionsDuration");
const routeStatus = document.getElementById("routeStatus");
const nextStop = document.getElementById("nextStop");
const pathList = document.getElementById("pathList");
const routeOverviewButton = document.getElementById("routeOverviewButton");
const directorySearch = document.getElementById("directorySearch");
const buildingFilter = document.getElementById("buildingFilter");
const floorFilter = document.getElementById("floorFilter");
const directoryList = document.getElementById("directoryList");
const directoryCount = document.getElementById("directoryCount");
const gpsStatus = document.getElementById("status");
const gpsStartButton = document.getElementById("startGPSButton");
const gpsStopButton = document.getElementById("stopGPSButton");
const nextClassRouteButton = document.getElementById("nextClassRouteButton");
const nextClassStatus = document.getElementById("nextClassStatus");
const scheduleDayFilter = document.getElementById("scheduleDayFilter");
const scheduleStatus = document.getElementById("scheduleStatus");
const scheduleList = document.getElementById("scheduleList");

// Shared map layers and state used by routing, GPS, the directory, and timetable.
const routeLayer = L.layerGroup().addTo(campusMap);
const placeLayer = L.layerGroup().addTo(campusMap);
let campusNodes = [];
let campusNodeById = new Map();
let campusLocations = [];
let timetableEntries = [];
let timetableVenueNodes = new Map();
let timetableVenueMappingError = null;
let pendingNextClassRoute = false;
let gpsWatchId = null;
let currentPosition = null;
let pendingGpsCenter = false;
// Heading-up navigation state.
let navigationMode = false;
let lastValidHeading = null;
let lastHeadingUpdate = 0;

const HEADING_UPDATE_INTERVAL = 150;
const MIN_MOVING_SPEED = 0.8; // metres per second
const TURN_ALERT_DISTANCE_METRES = 25;
let usingCurrentLocation = false;
let currentPositionMarker = null;
let gpsFallbackStartId = "";
let gpsStartOption = null;
let routeRequestController = null;
let lastRoutedPosition = null;
let routeTurnManeuvers = [];
let alertedTurnManeuverIds = new Set();
let turnAlertRouteKey = "";
const walkingRouteEndpoint =
  "https://routing.openstreetmap.de/routed-foot/route/v1/foot";
const timetableDays = {
  MON: "Monday",
  TUE: "Tuesday",
  WED: "Wednesday",
  THU: "Thursday",
  FRI: "Friday",
  SAT: "Saturday",
  SUN: "Sunday"
};
const timetableDayOrder = Object.keys(timetableDays);
const directorySources = [
  { path: "data/BTECH.csv", hasHeader: false },
  { path: "data/CSIT.csv", hasHeader: true },
  { path: "data/KP Nautiyal.csv", hasHeader: true },
  { path: "data/Mechanical.csv", hasHeader: true },
  { path: "data/Paramedical.csv", hasHeader: true }
];
const buildingEntranceIds = {
  BTech: "BTG",
  CSIT: "CS-F",
  "KP NAUTIYAL (College of Law)": "KP-B",
  "Mechanical (Vishwakarma)": "MB-1",
  Paramedical: "PM-L"
};

function formatScheduleTime(value) {
  const [hour, minute] = value.split(":").map(Number);
  const period = hour < 12 ? "AM" : "PM";
  return `${hour % 12 || 12}:${String(minute).padStart(2, "0")} ${period}`;
}

// Render the verified Group 1 and shared Section A timetable records.
function renderTimetable() {
  const selectedDay = scheduleDayFilter.value;
  const visibleEntries = timetableEntries.filter(
    (entry) => !selectedDay || entry.day === selectedDay
  );
  scheduleList.replaceChildren();

  if (!visibleEntries.length) {
    const emptyState = document.createElement("li");
    emptyState.textContent = "No classes are listed for this day.";
    scheduleList.appendChild(emptyState);
  }

  visibleEntries.forEach((entry) => {
    const item = document.createElement("li");
    const time = document.createElement("span");
    time.className = "time";
    time.textContent =
      `${formatScheduleTime(entry.start_time)} - ${formatScheduleTime(entry.end_time)}`;

    const details = document.createElement("div");
    details.className = "schedule-text";
    const title = document.createElement("strong");
    title.textContent = `${entry.course_code} - ${entry.subject_name}`;
    const metadata = document.createElement("span");
    metadata.textContent = [
      timetableDays[entry.day],
      entry.subject_type === "PRACTICAL" ? "Lab" : "Class",
      entry.venue,
      entry.faculty_name
    ].join(" · ");
    details.append(title, metadata);
    item.append(time, details);
    scheduleList.appendChild(item);
  });

  const dayDescription = selectedDay
    ? `${timetableDays[selectedDay]}`
    : "the week";
  scheduleStatus.textContent =
    `${visibleEntries.length} Group 1 and shared class/lab periods shown for ${dayDescription}.`;
}

async function loadTimetable() {
  const rows = await fetchCsv("data/timetable_examples.csv", true);
  timetableEntries = rows
    .filter(
      (row) =>
        row.record_type === "class" &&
        row.section === "A" &&
        row.semester === "3" &&
        ["G1", "BOTH"].includes(row.group_scope) &&
        row.is_example === "false"
    )
    .map((row, index) => {
      if (
        !timetableDays[row.day] ||
        !row.start_time ||
        !row.end_time ||
        !row.course_code ||
        !row.subject_name ||
        !row.subject_type ||
        !row.venue ||
        !row.faculty_name
      ) {
        throw new Error(`Timetable class record ${index + 1} is incomplete.`);
      }
      return row;
    })
    .sort(
      (left, right) =>
        timetableDayOrder.indexOf(left.day) - timetableDayOrder.indexOf(right.day) ||
        left.start_time.localeCompare(right.start_time)
    );

  if (!timetableEntries.length) {
    throw new Error("No verified Section A, Semester III, Group 1 classes were found.");
  }

  const availableDays = timetableDayOrder;
  scheduleDayFilter.replaceChildren(new Option("All days", ""));
  availableDays.forEach((day) => {
    scheduleDayFilter.appendChild(new Option(timetableDays[day], day));
  });
  const today = new Date().toLocaleDateString("en-US", {
    weekday: "short",
    timeZone: "Asia/Kolkata"
  }).toUpperCase();

  if (timetableDays[today] && availableDays.includes(today)) {
    scheduleDayFilter.value = today;
  }
  renderTimetable();
}

async function loadTimetableVenueNodes() {
  const rows = await fetchCsv("data/timetable_venue_nodes.csv", true);
  const venueNodes = new Map();
  rows.forEach((row, index) => {
    const venue = row.venue?.trim() || "";
    const nodeId = row.node_id?.trim() || "";
    if (!venue || !nodeId || !campusNodeById.has(nodeId) || venueNodes.has(venue)) {
      throw new Error(`Timetable venue mapping ${index + 2} is invalid.`);
    }
    venueNodes.set(venue, {
      nodeId,
      routeNote: row.route_note?.trim() || ""
    });
  });
  timetableVenueNodes = venueNodes;
  timetableVenueMappingError = null;
}

function findUpcomingClass(now = new Date()) {
  const parts = new Intl.DateTimeFormat("en-US", {
    timeZone: "Asia/Kolkata",
    weekday: "short",
    hour: "2-digit",
    minute: "2-digit",
    hourCycle: "h23"
  }).formatToParts(now);
  const campusTime = Object.fromEntries(
    parts.map(({ type, value }) => [type, value])
  );
  const today = campusTime.weekday.toUpperCase();
  const currentMinutes =
    Number(campusTime.hour) * 60 + Number(campusTime.minute);
  return timetableEntries.find(
    (entry) =>
      entry.day === today &&
      scheduleTimeInMinutes(entry.start_time) >= currentMinutes
  ) || null;
}

function scheduleTimeInMinutes(value) {
  const [hours, minutes] = value.split(":").map(Number);
  return hours * 60 + minutes;
}

function setNextClassStatus(message) {
  nextClassStatus.textContent = message;
  nextClassStatus.hidden = !message;
}

function routeToNextClass(now = new Date()) {
  if (!timetableEntries.length) {
    setNextClassStatus("The class schedule is unavailable right now.");
    return;
  }
  if (timetableVenueMappingError) {
    setNextClassStatus(
      `Next-class routing is unavailable: ${timetableVenueMappingError.message}`
    );
    return;
  }

  const nextClass = findUpcomingClass(now);
  if (!nextClass) {
    setNextClassStatus("No upcoming classes today.");
    return;
  }

  if (nextClass.venue.toLocaleLowerCase() === "online") {
    setNextClassStatus(
      `Your next class, ${nextClass.subject_name}, is online; no campus route is needed.`
    );
    return;
  }

  const venueRoute = timetableVenueNodes.get(nextClass.venue);
  const destination = venueRoute
    ? campusNodeById.get(venueRoute.nodeId)
    : null;
  if (!destination) {
    setNextClassStatus(
      `Your next class is ${nextClass.subject_name} at ${nextClass.venue}, but no walking route is mapped for that venue.`
    );
    return;
  }

  destinationSelect.value = destination.id;
  setNextClassStatus(
    `Next class: ${nextClass.subject_name} in ${nextClass.venue} at ${formatScheduleTime(nextClass.start_time)}. ${venueRoute.routeNote}`
  );

  if (!usingCurrentLocation && !startSelect.value) {
    if (gpsWatchId !== null) {
      usingCurrentLocation = true;
      startSelect.value = "__gps__";
      void updateRoute();
    } else if (navigator.geolocation) {
      pendingNextClassRoute = true;
      startGPS();
    } else {
      setNextClassStatus(
        "Choose a starting point to get directions; this browser does not support location services."
      );
      void updateRoute();
    }
  } else {
    void updateRoute();
  }
  document.getElementById("route-planner").scrollIntoView({ behavior: "smooth" });
}

// Parse CSV responses with Papa Parse and report malformed source files.
function parseCsv(text, source, header) {
  const result = Papa.parse(text, {
    header,
    skipEmptyLines: true,
    transformHeader: (name) => name.trim(),
    transform: (value) => value.trim()
  });
  if (result.errors.length) {
    throw new Error(
      `Could not parse ${source}: ${result.errors[0].message}`
    );
  }
  return result.data;
}

async function fetchCsv(path, header) {
  const response = await fetch(path);
  if (!response.ok) {
    throw new Error(`Could not load ${path} (HTTP ${response.status}).`);
  }
  return parseCsv(await response.text(), path, header);
}

async function fetchWalkingRoute(start, destination) {
  const coordinates = [
    `${start.lon},${start.lat}`,
    `${destination.lon},${destination.lat}`
  ].join(";");
  const url = new URL(
    `${walkingRouteEndpoint}/${coordinates}`,
    window.location.href
  );
  url.searchParams.set("overview", "full");
  url.searchParams.set("geometries", "geojson");
  url.searchParams.set("steps", "true");

  const response = await fetch(url, {
    signal: routeRequestController.signal
  });
  if (!response.ok) {
    throw new Error(`Walking route service returned HTTP ${response.status}.`);
  }
  const result = await response.json();
  if (result.code !== "Ok" || !result.routes?.length) {
    if (result.code === "NoRoute") return null;
    throw new Error(result.message || "Walking route could not be calculated.");
  }

  const route = result.routes[0];
  if (
    route.geometry?.type !== "LineString" ||
    !Array.isArray(route.geometry.coordinates) ||
    !route.geometry.coordinates.length ||
    !Array.isArray(route.legs) ||
    !Array.isArray(result.waypoints) ||
    result.waypoints.length !== 2 ||
    result.waypoints.some(
      (waypoint) =>
        !Array.isArray(waypoint.location) ||
        waypoint.location.length < 2 ||
        !Number.isFinite(waypoint.location[0]) ||
        !Number.isFinite(waypoint.location[1])
    ) ||
    !Number.isFinite(route.distance) ||
    !Number.isFinite(route.duration) ||
    route.geometry.coordinates.some(
      (coordinate) =>
        !Array.isArray(coordinate) ||
        coordinate.length < 2 ||
        !Number.isFinite(coordinate[0]) ||
        !Number.isFinite(coordinate[1])
    )
  ) {
    throw new Error("Walking route service returned invalid route data.");
  }
  return { ...route, waypoints: result.waypoints || [] };
}

// Load map coordinates and normalize the building-specific directory formats.
async function loadCampusData() {
  const [nodeRows, ...directoryRowSets] = await Promise.all([
    fetchCsv("data/node_coordinates.csv", true),
    ...directorySources.map((source) => fetchCsv(source.path, source.hasHeader))
  ]);

  campusNodes = nodeRows.map((row) => {
    const lat = Number(row.lat);
    const lon = Number(row.lon);
    if (!row.node_id || !row.name || !Number.isFinite(lat) || !Number.isFinite(lon)) {
      throw new Error("Campus coordinate data contains an invalid node.");
    }
    return {
      id: row.node_id,
      name: row.name,
      lat,
      lon
    };
  });
  campusNodeById = new Map(campusNodes.map((node) => [node.id, node]));
  if (campusNodeById.size !== campusNodes.length) {
    throw new Error("Campus coordinate data contains duplicate node IDs.");
  }

  campusLocations = directoryRowSets.flatMap((rows, sourceIndex) => {
    const source = directorySources[sourceIndex];
    return rows.map((row, rowIndex) => {
      let location;
      if (source.hasHeader) {
        location = {
          building: row.Block?.trim() || "",
          floor: row.Floor?.trim() || "",
          room: row["Room Number"]?.trim() || "",
          description: row["Room Name"]?.trim() || "",
          side: "",
          notes: row.Notes?.trim() || "",
          navigationNodeId: ""
        };
        if (!location.building) {
          throw new Error(
            `${source.path} has no building name at row ${rowIndex + 2}.`
          );
        }
      } else {
        if (row.length !== 5 && row.length !== 6) {
          throw new Error(
            `${source.path} row ${rowIndex + 1} must have five or six columns.`
          );
        }
        const [building, floor, room, description, side, navigationNodeId = ""] =
          row;
        location = {
          building,
          floor,
          room,
          description,
          side,
          notes: "",
          navigationNodeId
        };
      }
      if (
        location.navigationNodeId &&
        !campusNodeById.has(location.navigationNodeId)
      ) {
        const rowNumber = rowIndex + (source.hasHeader ? 2 : 1);
        throw new Error(
          `${source.path} has an unknown navigation node ID at row ${rowNumber}: ${location.navigationNodeId}.`
        );
      }
      return location;
    });
  });
}

function populateRouteLocations() {
  const places = campusNodes
    .sort((a, b) => a.name.localeCompare(b.name));
  [startSelect, destinationSelect].forEach((select) => {
    select.replaceChildren();
    select.appendChild(
      new Option(
        select === startSelect ? "Select starting point" : "Select destination",
        ""
      )
    );
    places.forEach((node) => {
      const option = document.createElement("option");
      option.value = node.id;
      option.textContent = node.name;
      select.appendChild(option);
    });
  });
}

// Draw campus locations and route geometry on their Leaflet layers.
function renderPlaces() {
  placeLayer.clearLayers();
  campusNodes.forEach((node) => {
    L.circleMarker([node.lat, node.lon], {
      radius: 6,
      color: "#07131f",
      weight: 2,
      fillColor: "#60d8ff",
      fillOpacity: 0.95
    })
      .bindTooltip(node.name)
      .on("click", () => {
        if (!startSelect.value && !usingCurrentLocation) {
          startSelect.value = node.id;
          gpsFallbackStartId = node.id;
        } else {
          destinationSelect.value = node.id;
        }
        updateRoute();
      })
      .addTo(placeLayer);
    });
  campusBounds = L.latLngBounds(campusNodes.map((node) => [node.lat, node.lon]));
  campusMap.fitBounds(campusBounds, { padding: [20, 20], maxZoom: homeZoom });
}

function renderPath(route, destination) {
  routeLayer.clearLayers();
  routeTurnManeuvers = [];
  // Make campus nodes nearly transparent while a route is active.
  placeLayer.eachLayer((layer) => {
    if (layer.setStyle) {
      layer.setStyle({
        opacity: 0.15,
        fillOpacity: 0.15
      });
    }
  });
  const routeCoordinates = route.geometry.coordinates.map(([lon, lat]) => [lat, lon]);
  activeRouteBounds = L.latLngBounds([
    ...routeCoordinates,
    [destination.lat, destination.lon],
    ...(usingCurrentLocation && currentPosition
      ? [[currentPosition.lat, currentPosition.lon]]
      : [])
  ]);
  const routeKey = `${usingCurrentLocation ? "__gps__" : startSelect.value}->${destination.id}`;
  if (routeKey !== activeRouteKey) {
    mapManuallyExploring = false;
    campusMap.fitBounds(activeRouteBounds, {
      padding: [36, 36],
      maxZoom: 18,
      animate: true
    });
    activeRouteKey = routeKey;
  }
  routeOverviewButton.disabled = false;
  L.polyline(routeCoordinates, {
    color: "#168bd2",
    weight: 6,
    opacity: 0.85
  }).addTo(routeLayer);
  if (routeCoordinates.length > 1) {
    navigationMode = usingCurrentLocation;
  }

  pathList.replaceChildren();
  if (route.distance === 0) {
    const item = document.createElement("li");
    item.classList.add("is-active");
    item.setAttribute("aria-current", "step");
    const icon = createManeuverIcon({ type: "arrive" }, true);
    const details = document.createElement("div");
    const instruction = document.createElement("strong");
    item.textContent = "You are already at this destination.";
    instruction.textContent = item.textContent;
    item.replaceChildren(icon, details);
    details.appendChild(instruction);
    pathList.appendChild(item);
    nextStop.textContent = destination.name;
    return;
  }

  const steps = route.legs.flatMap((leg) => leg.steps || []);
  if (!steps.length) {
    const item = document.createElement("li");
    const icon = createManeuverIcon({ type: "continue" });
    const details = document.createElement("div");
    const instruction = document.createElement("strong");
    item.classList.add("is-active");
    item.setAttribute("aria-current", "step");
    instruction.textContent = `Follow the mapped walking route to ${destination.name}.`;
    item.append(icon, details);
    details.appendChild(instruction);
    pathList.appendChild(item);
    nextStop.textContent = destination.name;
    return;
  }
  if (usingCurrentLocation) {
    routeTurnManeuvers = getTurnManeuvers(steps);
  }
  steps.forEach((stepData, index) => {
    if (stepData.distance < 1 && index !== steps.length - 1) return;
    const item = document.createElement("li");
    const isFinalStep = index === steps.length - 1;
    const isActiveStep = !pathList.querySelector(".is-active");
    const details = document.createElement("div");
    const text = document.createElement("strong");
    const distance = document.createElement("span");
    const maneuver = stepData.maneuver || { type: "continue" };
    const icon = createManeuverIcon(maneuver, isFinalStep);
    if (isActiveStep) {
      item.classList.add("is-active");
      item.setAttribute("aria-current", "step");
    }
    text.textContent = describeManeuver(
      maneuver,
      "",
      isFinalStep
    );
    distance.className = "distance-tag";
    distance.textContent = formatDistance(stepData.distance);
    details.appendChild(text);
    if (stepData.name?.trim()) {
      const stepLabel = document.createElement("small");
      stepLabel.textContent = stepData.name.trim();
      details.appendChild(stepLabel);
    }
    item.append(icon, details, distance);
    pathList.appendChild(item);
  });
  const nextStep = steps.find((step) => step.name);
  nextStop.textContent = nextStep?.name || destination.name;
  if (usingCurrentLocation && currentPosition) {
    checkUpcomingTurn(currentPosition);
  }
}

function createManeuverIcon(maneuver, isFinalStep = false) {
  const icon = document.createElement("span");
  const modifier = maneuver.modifier?.toLocaleLowerCase() || "";
  const icons = {
    left: "↰",
    "sharp left": "↶",
    "slight left": "↖",
    right: "↱",
    "sharp right": "↷",
    "slight right": "↗",
    straight: "↑"
  };
  icon.className = "maneuver-icon";
  icon.setAttribute("aria-hidden", "true");
  if (isFinalStep || maneuver.type === "arrive") {
    icon.classList.add("destination-icon");
    icon.textContent = "📍";
  } else if (maneuver.type === "uturn") {
    icon.textContent = modifier.includes("left") ? "↶" : "↷";
  } else if (maneuver.type === "roundabout" || maneuver.type === "rotary") {
    icon.textContent = "⟳";
  } else if (maneuver.type === "merge" || maneuver.type === "off ramp") {
    icon.textContent = icons[modifier] || "↗";
  } else {
    icon.textContent = icons[modifier] || "↑";
  }
  return icon;
}

function describeManeuver(maneuver, roadName, isFinalStep) {
  const name = roadName ? ` onto ${roadName}` : "";
  if (isFinalStep || maneuver.type === "arrive") {
    return `Arrive at ${destinationSelect.selectedOptions[0]?.textContent || "destination"}`;
  }
  if (maneuver.type === "depart") {
    return `Start walking${name}`;
  }
  if (maneuver.type === "roundabout" || maneuver.type === "rotary") {
    return `Enter the roundabout${name}`;
  }
  if (maneuver.type === "turn" && maneuver.modifier) {
    return `Turn ${maneuver.modifier}${name}`;
  }
  if (maneuver.type === "uturn") {
    return `Make a U-turn${name}`;
  }
  if (maneuver.type === "fork" && maneuver.modifier) {
    return `Keep ${maneuver.modifier}${name}`;
  }
  if (maneuver.type === "merge" && maneuver.modifier) {
    return `Merge ${maneuver.modifier}${name}`;
  }
  if (maneuver.type === "end of road" && maneuver.modifier) {
    return `At the end of the path, turn ${maneuver.modifier}${name}`;
  }
  if (maneuver.type === "off ramp" && maneuver.modifier) {
    return `Take the ${maneuver.modifier} ramp${name}`;
  }
  if (maneuver.type === "continue" || maneuver.type === "new name") {
    return `Continue walking${name}`;
  }
  return `${maneuver.type || "Continue walking"}${name}`;
}

function formatDistance(distance) {
  return distance >= 1000
    ? `${(distance / 1000).toFixed(1)} km`
    : `${Math.round(distance)} m`;
}

function getTurnManeuvers(steps) {
  return steps.flatMap((step) => {
    const maneuver = step.maneuver;
    const location = maneuver?.location;
    if (
      !isTurnManeuver(maneuver) ||
      !Array.isArray(location) ||
      location.length < 2 ||
      !Number.isFinite(location[0]) ||
      !Number.isFinite(location[1])
    ) {
      return [];
    }

    const [lon, lat] = location;
    return [{
      id: `${maneuver.type}:${maneuver.modifier || ""}:${lon.toFixed(5)}:${lat.toFixed(5)}`,
      lat,
      lon,
      instruction: describeManeuver(maneuver, step.name, false)
    }];
  });
}

function isTurnManeuver(maneuver) {
  return [
    "turn",
    "uturn",
    "fork",
    "roundabout",
    "rotary",
    "merge",
    "end of road",
    "off ramp"
  ].includes(maneuver?.type);
}

function checkUpcomingTurn(position) {
  const nextTurn = routeTurnManeuvers.find(
    (maneuver) => !alertedTurnManeuverIds.has(maneuver.id)
  );
  if (!nextTurn) return;

  const distance = getDistanceInMetres(
    position.lat,
    position.lon,
    nextTurn.lat,
    nextTurn.lon
  );
  if (distance > TURN_ALERT_DISTANCE_METRES) return;

  alertedTurnManeuverIds.add(nextTurn.id);
  routeStatus.textContent =
    `${nextTurn.instruction} in ${formatDistance(distance)}.`;
  if (typeof navigator.vibrate === "function") {
    navigator.vibrate([140, 80, 140]);
  }
}

function clearTurnAlerts() {
  routeTurnManeuvers = [];
  alertedTurnManeuverIds.clear();
  turnAlertRouteKey = "";
}

function renderGpsMarker(position, label) {
  const latLng = [position.lat, position.lon];
  if (currentPositionMarker) {
    currentPositionMarker.setLatLng(latLng);
  } else {
    currentPositionMarker = L.circleMarker(latLng, {
      radius: 8,
      color: "#ffffff",
      weight: 2,
      fillColor: "#e44848",
      fillOpacity: 1
    }).addTo(campusMap);
  }
  currentPositionMarker.bindTooltip(label);
}

async function updateRoute() {
  const selectedStart = campusNodeById.get(startSelect.value);
  const destination = campusNodeById.get(destinationSelect.value);
  if ((!selectedStart && !usingCurrentLocation) || !destination) {
    clearTurnAlerts();
    routeTitle.textContent = "Choose a route";
    routeStatus.textContent = !selectedStart && !usingCurrentLocation
      ? destination
        ? "Select a starting point to continue."
        : "Select a starting point and destination."
      : `Starting point set to ${
          usingCurrentLocation ? "your location" : selectedStart.name
        }. Select a destination.`;
    routeDuration.textContent = "--";
    routeDistance.textContent = "--";
    directionsDistance.textContent = "--";
    directionsDuration.textContent = "--";
    nextStop.textContent = "--";
    document.getElementById("navigationDestination").textContent =
      "Choose one!";
    document.getElementById("navigationEta").textContent = "--";
    document.getElementById("navigationDistance").textContent = "--";
    pathList.replaceChildren();
    routeLayer.clearLayers();
    activeRouteKey = "";
    activeRouteBounds = null;
    routeOverviewButton.disabled = true;
    navigationMode = false;
    // Restore normal campus node visibility.
    placeLayer.eachLayer((layer) => {
      if (layer.setStyle) {
        layer.setStyle({
          opacity: 1,
          fillOpacity: 0.95
        });
      }
    });
    routeRequestController?.abort();
    return;
  }

  const start = usingCurrentLocation ? currentPosition : selectedStart;
  if (!start) {
    routeRequestController?.abort();
    routeStatus.textContent = "Waiting for your GPS location…";
    return;
  }

  const turnRouteKey =
    `${usingCurrentLocation ? "__gps__" : selectedStart.id}->${destination.id}`;
  if (turnRouteKey !== turnAlertRouteKey) {
    alertedTurnManeuverIds.clear();
    turnAlertRouteKey = turnRouteKey;
  }
  routeTurnManeuvers = [];
  routeRequestController?.abort();
  routeRequestController = new AbortController();
  const requestController = routeRequestController;
  routeTitle.textContent =
    `${usingCurrentLocation ? "My location" : selectedStart.name} → ${destination.name}`;
  routeStatus.textContent = "Finding a walking route along mapped paths…";

  try {
    const route = await fetchWalkingRoute(start, destination);
    if (requestController !== routeRequestController) return;
    if (!route) {
      clearTurnAlerts();
      routeStatus.textContent =
        "No walking route is available for these locations.";
      routeDuration.textContent = "--";
      routeDistance.textContent = "--";
      directionsDistance.textContent = "--";
      directionsDuration.textContent = "--";
      nextStop.textContent = "--";
      document.getElementById("navigationDestination").textContent =
        destination.name;
      document.getElementById("navigationEta").textContent = "--";
      document.getElementById("navigationDistance").textContent = "--";
      pathList.replaceChildren();
      routeLayer.clearLayers();
      activeRouteKey = "";
      activeRouteBounds = null;
      routeOverviewButton.disabled = true;
      navigationMode = false;
      return;
    }

    const snappedStart = route.waypoints[0].location;
    const snappedDestination = route.waypoints[1].location;
    const startOffset = getDistanceInMetres(
      start.lat,
      start.lon,
      snappedStart[1],
      snappedStart[0]
    );
    const destinationOffset = getDistanceInMetres(
      destination.lat,
      destination.lon,
      snappedDestination[1],
      snappedDestination[0]
    );
    const duration = Math.ceil(route.duration / 60);
    routeStatus.textContent =
      `Walking route follows mapped paths; start snapped ${Math.round(startOffset)} m and destination ${Math.round(destinationOffset)} m to a path.`;
    routeDuration.textContent = `${duration} min`;
    routeDistance.textContent = formatDistance(route.distance);
    directionsDistance.textContent = formatDistance(route.distance);
    directionsDuration.textContent = `${duration} min`;
    document.getElementById("navigationDestination").textContent =
      destination.name;
    document.getElementById("navigationEta").textContent = `${duration} min`;
    document.getElementById("navigationDistance").textContent =
      formatDistance(route.distance);

    if (usingCurrentLocation) {
      const [snappedLon, snappedLat] = snappedStart;
      const snappedPosition = { lat: snappedLat, lon: snappedLon };
      renderGpsMarker(
        snappedPosition,
        "Your location · snapped to walking route"
      );
      lastRoutedPosition = start;
      gpsStatus.textContent =
        `Walking route starts at the nearest mapped path (${Math.round(
          startOffset
        )} m from GPS).`;
    }
    renderPath(route, destination);
  } catch (error) {
    if (
      error.name === "AbortError" ||
      requestController !== routeRequestController
    ) {
      return;
    }
    console.error("Could not calculate walking route:", error);
    clearTurnAlerts();
    routeStatus.textContent =
      "Could not load a road route. Check your internet connection and try again.";
    routeDuration.textContent = "--";
    routeDistance.textContent = "--";
    directionsDistance.textContent = "--";
    directionsDuration.textContent = "--";
    nextStop.textContent = "--";
    document.getElementById("navigationEta").textContent = "--";
    document.getElementById("navigationDistance").textContent = "--";
    pathList.replaceChildren();
    routeLayer.clearLayers();
    activeRouteKey = "";
    activeRouteBounds = null;
    routeOverviewButton.disabled = true;
    navigationMode = false;
  }
}

// Measure the distance between two GPS coordinates in metres.
function getDistanceInMetres(lat1, lon1, lat2, lon2) {
  const radians = (degrees) => (degrees * Math.PI) / 180;
  const latitudeDifference = radians(lat2 - lat1);
  const longitudeDifference = radians(lon2 - lon1);
  const value =
    Math.sin(latitudeDifference / 2) ** 2 +
    Math.cos(radians(lat1)) *
      Math.cos(radians(lat2)) *
      Math.sin(longitudeDifference / 2) ** 2;
  return 6371000 * 2 * Math.atan2(Math.sqrt(value), Math.sqrt(1 - value));
}

function updateMapHeading(heading) {
  if (!Number.isFinite(heading)) return;

  const now = performance.now();

  if (now - lastHeadingUpdate < HEADING_UPDATE_INTERVAL) {
    return;
  }

  lastHeadingUpdate = now;
  lastValidHeading = heading;

  // The direction of travel will always point toward the top.
  campusMap.setHeading(heading, {
    ease: 0.2,
    deadzone: 1
  });
}

function startGPS() {
  if (!navigator.geolocation) {
    gpsStatus.textContent = "This browser does not support location services.";
    return;
  }
  if (gpsWatchId !== null) {
    usingCurrentLocation = true;
    startSelect.value = "__gps__";
    void updateRoute();
    return;
  }

  gpsFallbackStartId = startSelect.value;
  usingCurrentLocation = true;
  gpsStartOption?.remove();
  gpsStartOption = new Option("My location", "__gps__");
  startSelect.appendChild(gpsStartOption);
  startSelect.value = "__gps__";
  gpsStatus.textContent = "Waiting for location permission…";
  gpsStartButton.disabled = true;
  gpsWatchId = navigator.geolocation.watchPosition(
    (position) => {
      pendingNextClassRoute = false;
      currentPosition = {
        lat: position.coords.latitude,
        lon: position.coords.longitude
      };
      gpsStatus.textContent = "Location tracking is active.";
      if (pendingGpsCenter) {
        pendingGpsCenter = false;
        centerMapOnCurrentPosition();
      }

      // Rotate the map according to GPS movement direction.
      const heading = position.coords.heading;
      const speed = position.coords.speed;

      if (
        heading !== null &&
        Number.isFinite(heading) &&
        speed !== null &&
        Number.isFinite(speed) &&
        speed >= MIN_MOVING_SPEED
      ) {
        updateMapHeading(heading);
      }

      document.getElementById("latitude").textContent =
        `Latitude: ${currentPosition.lat.toFixed(6)}`;
      document.getElementById("longitude").textContent =
        `Longitude: ${currentPosition.lon.toFixed(6)}`;
      document.getElementById("accuracy").textContent =
        `Accuracy: approximately ${Math.round(position.coords.accuracy)} metres`;
      const movedDistance = lastRoutedPosition
        ? getDistanceInMetres(
            currentPosition.lat,
            currentPosition.lon,
            lastRoutedPosition.lat,
            lastRoutedPosition.lon
          )
        : Infinity;
      if (!lastRoutedPosition) {
        renderGpsMarker(currentPosition, "Your GPS location");
      }
      if (usingCurrentLocation && movedDistance >= 15) {
        void updateRoute();
      } else if (usingCurrentLocation) {
        checkUpcomingTurn(currentPosition);
      }
    },
    (error) => {
      gpsStatus.textContent = `Location error: ${error.message}`;
      if (pendingNextClassRoute) {
        pendingNextClassRoute = false;
        setNextClassStatus(
          "Could not get your location. Choose a starting point or check location permissions."
        );
      }
      if (gpsWatchId !== null) {
        navigator.geolocation.clearWatch(gpsWatchId);
      }
      gpsWatchId = null;
      usingCurrentLocation = false;
      lastRoutedPosition = null;
      pendingGpsCenter = false;
      navigationMode = false;

      // Stop heading-up mode and restore north.
      campusMap.stopHeadingUp();
      campusMap.setBearing(0);

      lastValidHeading = null;
      lastHeadingUpdate = 0;
      gpsStartOption?.remove();
      gpsStartOption = null;
      startSelect.value = gpsFallbackStartId;
      gpsStartButton.disabled = false;
      gpsStopButton.disabled = true;
      void updateRoute();
    },
    { enableHighAccuracy: true, maximumAge: 10000, timeout: 15000 }
  );
  gpsStopButton.disabled = false;
}

function centerMapOnCurrentPosition() {
  if (!currentPosition) return;
  campusMap.setView(
    [currentPosition.lat, currentPosition.lon],
    Math.max(campusMap.getZoom(), 18),
    { animate: true }
  );
}

function showRouteOverview() {
  if (!activeRouteBounds) return;
  mapManuallyExploring = false;
  campusMap.stopHeadingUp();
  campusMap.setBearing(0);
  campusMap.fitBounds(activeRouteBounds, {
    padding: [36, 36],
    maxZoom: 18,
    animate: true
  });
}

function goToGPSLocation() {
  if (gpsWatchId !== null && currentPosition) {
    centerMapOnCurrentPosition();
    return;
  }
  if (!navigator.geolocation) {
    startGPS();
    return;
  }
  pendingGpsCenter = true;
  startGPS();
}

function stopGPS() {
  if (gpsWatchId === null) return;
  navigator.geolocation.clearWatch(gpsWatchId);
  gpsWatchId = null;
  if (pendingNextClassRoute) {
    pendingNextClassRoute = false;
    setNextClassStatus(
      "Location tracking stopped. Choose a starting point to route to your next class."
    );
  }
  usingCurrentLocation = false;
  lastRoutedPosition = null;
  pendingGpsCenter = false;
  navigationMode = false;

  // Stop heading-up mode and restore north.
  campusMap.stopHeadingUp();
  campusMap.setBearing(0);

  lastValidHeading = null;
  lastHeadingUpdate = 0;
  gpsStartOption?.remove();
  gpsStartOption = null;
  startSelect.value = gpsFallbackStartId;
  gpsStartButton.disabled = false;
  gpsStopButton.disabled = true;
  gpsStatus.textContent = "Location tracking stopped.";
  routeRequestController?.abort();
  void updateRoute();
}

function normalizeLocationName(value) {
  return value.toLocaleLowerCase().replace(/[^a-z0-9]/g, "");
}

// Match a directory record to a map node or its building entrance.
function findDirectoryDestination(location) {
  const mappedNode = location.navigationNodeId
    ? campusNodeById.get(location.navigationNodeId)
    : null;
  if (mappedNode) return { node: mappedNode, isBuildingEntrance: false };

  const candidateNames = [
    location.room,
    location.description,
    location.notes
  ].filter(Boolean);
  for (const candidate of candidateNames) {
    const normalized = normalizeLocationName(candidate);
    const match = campusNodes.find(
      (node) => normalizeLocationName(node.name) === normalized
    );
    if (match) return { node: match, isBuildingEntrance: false };
  }
  const entranceId = buildingEntranceIds[location.building];
  const entrance = entranceId ? campusNodeById.get(entranceId) : null;
  return entrance ? { node: entrance, isBuildingEntrance: true } : null;
}

function renderBuildingDirectory() {
  const query = directorySearch.value.trim().toLocaleLowerCase();
  const selectedBuilding = buildingFilter.value;
  const selectedFloor = floorFilter.value;
  const matchingLocations = campusLocations.filter((location) => {
    const matchesBuilding =
      !selectedBuilding || location.building === selectedBuilding;
    const matchesFloor = !selectedFloor || location.floor === selectedFloor;
    const searchableText = [
      location.building,
      location.floor,
      location.room,
      location.description,
      location.side,
      location.notes,
      location.navigationNodeId
    ]
      .join(" ")
      .toLocaleLowerCase();
    return (
      matchesBuilding &&
      matchesFloor &&
      searchableText.includes(query)
    );
  });

  directoryCount.textContent =
    `${matchingLocations.length} of ${campusLocations.length} locations`;
  directoryList.replaceChildren();
  if (!matchingLocations.length) {
    const emptyState = document.createElement("li");
    emptyState.textContent = "No rooms or facilities match this filter.";
    directoryList.appendChild(emptyState);
    return;
  }

  matchingLocations.forEach((location) => {
    const item = document.createElement("li");
    const name = document.createElement("strong");
    const details = document.createElement("span");
    const heading = document.createElement("div");
    heading.className = "directory-item-heading";
    const destination = findDirectoryDestination(location);
    name.textContent =
      location.room || location.description || location.building;
    heading.appendChild(name);
    details.textContent = [
      location.building,
      location.floor,
      location.side,
      location.building === "BTech" &&
      location.floor.startsWith("Ground") &&
      location.navigationNodeId
        ? `Access via ${location.navigationNodeId}`
        : "",
      location.room && location.description ? location.description : "",
      location.notes
    ]
      .filter(Boolean)
      .join(" · ");
    if (destination) {
      const routeButton = document.createElement("button");
      routeButton.className = "directory-route-button";
      routeButton.type = "button";
      const routeLabel = location.navigationNodeId
        ? `Route via ${location.navigationNodeId}`
        : destination.isBuildingEntrance
          ? `Route to ${location.building} entrance`
          : `Route to ${destination.node.name}`;
      routeButton.textContent = "↗";
      routeButton.setAttribute("aria-label", routeLabel);
      routeButton.title = routeLabel;
      routeButton.addEventListener("click", () => {
        destinationSelect.value = destination.node.id;
        updateRoute();
        document.getElementById("routes").scrollIntoView({ behavior: "smooth" });
      });
      heading.appendChild(routeButton);
    }
    item.append(heading, details);
    directoryList.appendChild(item);
  });
}

function populateDirectoryFilters() {
  const buildings = [...new Set(
    campusLocations.map((location) => location.building).filter(Boolean)
  )].sort((a, b) => a.localeCompare(b));
  buildingFilter.replaceChildren(new Option("All buildings", ""));
  buildings.forEach((building) => {
    buildingFilter.appendChild(new Option(building, building));
  });

  const floors = [...new Set(
    campusLocations.map((location) => location.floor).filter(Boolean)
  )].sort((a, b) => a.localeCompare(b));
  floorFilter.replaceChildren(new Option("All floors", ""));
  floors.forEach((floor) => {
    floorFilter.appendChild(new Option(floor, floor));
  });
}

// Load the datasets and initialize the map, timetable, and directory UI.
async function initializeCampusApp() {
  try {
    gpsStatus.textContent = "Loading campus data…";
    await loadCampusData();
    try {
      await loadTimetable();
    } catch (error) {
      console.error("Could not load the timetable:", error);
      scheduleStatus.textContent = `Could not load the timetable: ${error.message}`;
      scheduleList.replaceChildren();
    }
    try {
      await loadTimetableVenueNodes();
    } catch (error) {
      console.error("Could not load timetable venue mappings:", error);
      timetableVenueMappingError = error;
    }
    populateRouteLocations();
    populateDirectoryFilters();
    renderPlaces();
    renderBuildingDirectory();
    updateRoute();
    gpsStatus.textContent =
      `Loaded ${campusNodes.length} mapped points`;
  } catch (error) {
    console.error("Could not initialize campus data:", error);
    gpsStatus.textContent = `Could not load campus data: ${error.message}`;
    routeStatus.textContent = "Campus route data could not be loaded.";
    directoryCount.textContent = "Data unavailable";
    directoryList.replaceChildren();
    const errorItem = document.createElement("li");
    errorItem.textContent = `Could not load the campus directory: ${error.message}`;
    directoryList.appendChild(errorItem);
  }
}

routeForm.addEventListener("submit", (event) => {
  event.preventDefault();
  updateRoute();
  document.getElementById("campus-map").scrollIntoView({ behavior: "smooth" });
});
startSelect.addEventListener("change", () => {
  usingCurrentLocation = startSelect.value === "__gps__";
  if (pendingNextClassRoute && !usingCurrentLocation) {
    pendingNextClassRoute = false;
  }
  if (!usingCurrentLocation) {
    gpsFallbackStartId = startSelect.value;
  }
  void updateRoute();
});
destinationSelect.addEventListener("change", () => void updateRoute());
routeOverviewButton.addEventListener("click", showRouteOverview);
directorySearch.addEventListener("input", renderBuildingDirectory);
buildingFilter.addEventListener("change", renderBuildingDirectory);
floorFilter.addEventListener("change", renderBuildingDirectory);
scheduleDayFilter.addEventListener("change", renderTimetable);
gpsStartButton.addEventListener("click", startGPS);
gpsStopButton.addEventListener("click", stopGPS);
nextClassRouteButton.addEventListener("click", () => routeToNextClass());

initializeCampusApp();
