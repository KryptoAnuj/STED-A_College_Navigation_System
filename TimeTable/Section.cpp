#include "Section.h"

Section::Section(std::string sectionName, std::string groupLabel, Venue defaultVenue,
                  std::string coordinator1, std::string coordinator2, int g1Cutoff)
    : sectionName(sectionName), groupLabel(groupLabel), defaultVenue(defaultVenue),
      coordinator1(coordinator1), coordinator2(coordinator2), g1Cutoff(g1Cutoff) {}

std::string Section::getSectionName() const { return sectionName; }
std::string Section::getGroupLabel() const { return groupLabel; }
Venue Section::getDefaultVenue() const { return defaultVenue; }
std::string Section::getCoordinator1() const { return coordinator1; }
std::string Section::getCoordinator2() const { return coordinator2; }
int Section::getG1Cutoff() const { return g1Cutoff; }

GroupScope Section::resolveGroup(int rollNumber) const {
    // Roll numbers from 1 up to and including the cutoff are G1;
    // everything after that is G2. (The cutoff itself varies per
    // section — confirmed in the real timetables, e.g. 40 vs 42.)
    if (rollNumber >= 1 && rollNumber <= g1Cutoff) {
        return GroupScope::G1;
    }
    return GroupScope::G2;
}

bool Section::createTimetableForSemester(int semesterNumber) {
    // map::find tells us if a Timetable already exists for this
    // semester, so we don't accidentally overwrite one.
    if (semesterTimetables.find(semesterNumber) != semesterTimetables.end()) {
        return false; // already exists
    }
    semesterTimetables.emplace(semesterNumber, Timetable(semesterNumber));
    return true;
}

Timetable* Section::getTimetable(int semesterNumber) {
    auto it = semesterTimetables.find(semesterNumber);
    if (it == semesterTimetables.end()) {
        return nullptr;
    }
    return &(it->second);
}

const Timetable* Section::getTimetable(int semesterNumber) const {
    auto it = semesterTimetables.find(semesterNumber);
    if (it == semesterTimetables.end()) {
        return nullptr;
    }
    return &(it->second);
}
