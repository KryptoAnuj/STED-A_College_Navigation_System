#include "User.h"

// =======================================================================
// User (abstract base)
// =======================================================================

User::User(std::string name, std::string userId) : name(name), userId(userId) {}

User::~User() {}

std::string User::getName() const { return name; }
std::string User::getUserId() const { return userId; }

// Note: getRole() has no body here — it's pure virtual in the header,
// so only Admin and Student implement it below.

// =======================================================================
// Admin
// =======================================================================

Admin::Admin(std::string name, std::string userId) : User(name, userId) {}

std::string Admin::getRole() const { return "Admin"; }

bool Admin::createTimetableForSection(Section& section, int semesterNumber) {
    return section.createTimetableForSemester(semesterNumber);
}

bool Admin::addClassEntry(Section& section, int semesterNumber,
                           const ScheduleEntry& entry, const std::string& timestamp) {
    Timetable* timetable = section.getTimetable(semesterNumber);
    if (timetable == nullptr) {
        return false; // that semester's timetable doesn't exist yet
    }
    return timetable->addEntry(entry, userId, timestamp);
}

bool Admin::editClassVenue(Section& section, int semesterNumber,
                            const TimeSlot& slot, GroupScope scope,
                            const Venue& newVenue, const std::string& timestamp) {
    Timetable* timetable = section.getTimetable(semesterNumber);
    if (timetable == nullptr) {
        return false;
    }
    return timetable->editEntryVenue(slot, scope, newVenue, userId, timestamp);
}

bool Admin::deleteClassEntry(Section& section, int semesterNumber,
                              const TimeSlot& slot, GroupScope scope,
                              const std::string& timestamp) {
    Timetable* timetable = section.getTimetable(semesterNumber);
    if (timetable == nullptr) {
        return false;
    }
    return timetable->deleteEntry(slot, scope, userId, timestamp);
}

bool Admin::addTemporaryOverride(Section& section, int semesterNumber,
                                  const TemporaryOverride& override,
                                  const std::string& timestamp) {
    Timetable* timetable = section.getTimetable(semesterNumber);
    if (timetable == nullptr) {
        return false;
    }
    timetable->addOverride(override, userId, timestamp);
    return true;
}

// =======================================================================
// Student
// =======================================================================

Student::Student(std::string name, std::string userId, Section* section, int rollNumber)
    : User(name, userId), section(section), rollNumber(rollNumber) {}

std::string Student::getRole() const { return "Student"; }

Section* Student::getSection() const { return section; }
int Student::getRollNumber() const { return rollNumber; }

GroupScope Student::getGroup() const {
    return section->resolveGroup(rollNumber);
}
