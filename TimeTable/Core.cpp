#include "Core.h"

// =======================================================================
// TimeSlot
// =======================================================================

TimeSlot::TimeSlot(std::string day, int startHour, int startMinute,
                    int endHour, int endMinute)
    : day(day), startHour(startHour), startMinute(startMinute),
      endHour(endHour), endMinute(endMinute) {}

std::string TimeSlot::getDay() const { return day; }
int TimeSlot::getStartHour() const { return startHour; }
int TimeSlot::getStartMinute() const { return startMinute; }
int TimeSlot::getEndHour() const { return endHour; }
int TimeSlot::getEndMinute() const { return endMinute; }

bool TimeSlot::operator==(const TimeSlot& other) const {
    return day == other.day &&
           startHour == other.startHour &&
           startMinute == other.startMinute &&
           endHour == other.endHour &&
           endMinute == other.endMinute;
}

bool TimeSlot::overlaps(const TimeSlot& other) const {
    // Different days can never overlap.
    if (day != other.day) {
        return false;
    }

    // Convert both slots' start/end into "minutes since midnight" so
    // they're easy to compare as plain numbers.
    int thisStart = startHour * 60 + startMinute;
    int thisEnd   = endHour * 60 + endMinute;
    int otherStart = other.startHour * 60 + other.startMinute;
    int otherEnd   = other.endHour * 60 + other.endMinute;

    // Two ranges overlap unless one ends before the other starts.
    return thisStart < otherEnd && otherStart < thisEnd;
}

// =======================================================================
// Venue
// =======================================================================

Venue::Venue(std::string name) : name(name) {}

std::string Venue::getName() const { return name; }

bool Venue::operator==(const Venue& other) const {
    return name == other.name;
}

// =======================================================================
// Subject (abstract base)
// =======================================================================

Subject::Subject(std::string courseCode, std::string subjectName,
                  std::string facultyName, int semesterNumber)
    : courseCode(courseCode), subjectName(subjectName),
      facultyName(facultyName), semesterNumber(semesterNumber) {}

Subject::~Subject() {}

std::string Subject::getCourseCode() const { return courseCode; }
std::string Subject::getSubjectName() const { return subjectName; }
std::string Subject::getFacultyName() const { return facultyName; }
int Subject::getSemesterNumber() const { return semesterNumber; }

// Note: describe() has no body here — it's pure virtual in the header
// (the "= 0"), so Subject itself never implements it. Only the derived
// classes below do.

// =======================================================================
// TheorySubject
// =======================================================================

TheorySubject::TheorySubject(std::string courseCode, std::string subjectName,
                              std::string facultyName, int semesterNumber)
    : Subject(courseCode, subjectName, facultyName, semesterNumber) {}

std::string TheorySubject::describe() const {
    return "[Theory] " + subjectName + " (" + courseCode + ") - " + facultyName;
}

// =======================================================================
// PracticalSubject
// =======================================================================

PracticalSubject::PracticalSubject(std::string courseCode, std::string subjectName,
                                    std::string facultyName, int semesterNumber)
    : Subject(courseCode, subjectName, facultyName, semesterNumber) {}

std::string PracticalSubject::describe() const {
    return "[Lab] " + subjectName + " (" + courseCode + ") - " + facultyName;
}
