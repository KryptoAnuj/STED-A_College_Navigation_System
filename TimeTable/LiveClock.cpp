#include "LiveClock.h"
#include <ctime>

// =======================================================================
// SystemClock
// =======================================================================

std::string SystemClock::getCurrentDay() const {
    // Ask the OS for the current time, then convert it to day-of-week.
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    // tm_wday is 0=Sunday, 1=Monday, ... 6=Saturday
    static const std::string dayNames[7] = {
        "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
    };
    return dayNames[localTime->tm_wday];
}

int SystemClock::getCurrentHour() const {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    return localTime->tm_hour;
}

int SystemClock::getCurrentMinute() const {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    return localTime->tm_min;
}

std::string SystemClock::getCurrentDate() const {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    // Build "YYYY-MM-DD" manually. tm_year is years since 1900,
    // tm_mon is 0-11, so both need adjusting.
    int year = localTime->tm_year + 1900;
    int month = localTime->tm_mon + 1;
    int day = localTime->tm_mday;

    std::string result = std::to_string(year) + "-";
    if (month < 10) result += "0";
    result += std::to_string(month) + "-";
    if (day < 10) result += "0";
    result += std::to_string(day);

    return result;
}

// =======================================================================
// LiveClassResolver
// =======================================================================

CurrentClassInfo LiveClassResolver::resolve(Section& section, int semesterNumber,
                                             const std::string& day, int hour, int minute,
                                             const std::string& date, GroupScope studentGroup) {
    CurrentClassInfo info;
    info.found = false;
    info.cancelled = false;

    Timetable* timetable = section.getTimetable(semesterNumber);
    if (timetable == nullptr) {
        return info; // no timetable exists for this semester at all
    }

    std::shared_ptr<Subject> resolvedSubject;
    Venue resolvedVenue;
    bool cancelled = false;

    bool wasFound = timetable->resolveClassAt(day, hour, minute, date, studentGroup,
                                               resolvedSubject, resolvedVenue, cancelled);

    if (!wasFound) {
        return info; // free period, or this slot falls in a skipped category
    }

    info.found = true;
    info.cancelled = cancelled;
    info.subjectName = resolvedSubject->getSubjectName();
    info.venueName = resolvedVenue.getName();
    info.facultyName = resolvedSubject->getFacultyName();

    return info;
}
