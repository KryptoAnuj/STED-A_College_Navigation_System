#ifndef SECTION_H
#define SECTION_H

#include <string>
#include <map>
#include "Core.h"       // Venue
#include "Timetable.h"  // Timetable

// ---------------------------------------------------------------------
// Section
// Represents one section (e.g. "O", "BIO-A"): its identity, default
// classroom, coordinators, the G1/G2 roll-number cutoff, and a map
// from semester number -> that semester's Timetable.
//
// Group (Electrical/Electronics/Biotech) is stored purely as a display
// label — it does NOT affect which subjects a section studies. That
// logic (the swap mechanism) is explicitly out of scope for now.
// ---------------------------------------------------------------------
class Section {
private:
    std::string sectionName;        // e.g. "O", "BIO-A"
    std::string groupLabel;         // "Electrical" / "Electronics" / "Biotech" — informational only
    Venue defaultVenue;
    std::string coordinator1;
    std::string coordinator2;
    int g1Cutoff;                   // last roll number belonging to G1 (e.g. 40 or 42)

    std::map<int, Timetable> semesterTimetables; // semester number -> Timetable

public:
    Section(std::string sectionName, std::string groupLabel, Venue defaultVenue,
            std::string coordinator1, std::string coordinator2, int g1Cutoff);

    std::string getSectionName() const;
    std::string getGroupLabel() const;
    Venue getDefaultVenue() const;
    std::string getCoordinator1() const;
    std::string getCoordinator2() const;
    int getG1Cutoff() const;

    // Given a roll number, returns which group that student is in.
    GroupScope resolveGroup(int rollNumber) const;

    // Creates a new, empty Timetable for the given semester if one
    // doesn't already exist. Returns false if it already exists
    // (use getTimetable + the Timetable's own methods to edit it).
    bool createTimetableForSemester(int semesterNumber);

    // Returns a pointer so the caller can call mutating methods on it.
    // Returns nullptr if no Timetable exists for that semester yet.
    Timetable* getTimetable(int semesterNumber);
    const Timetable* getTimetable(int semesterNumber) const;
};

#endif // SECTION_H
