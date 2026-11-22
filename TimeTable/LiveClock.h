#ifndef LIVE_CLOCK_H
#define LIVE_CLOCK_H

#include <string>
#include "Section.h" // Section, GroupScope, Timetable

// ---------------------------------------------------------------------
// SystemClock
// Thin wrapper around C++'s built-in time functions. The rest of the
// program never calls raw system time functions directly — it always
// goes through this class, which keeps the "real time" source in one
// place and easy to swap out later if needed.
// ---------------------------------------------------------------------
class SystemClock {
public:
    std::string getCurrentDay() const;    // "MON", "TUE", ... "SUN"
    int getCurrentHour() const;           // 0-23
    int getCurrentMinute() const;         // 0-59
    std::string getCurrentDate() const;   // "YYYY-MM-DD"
};

// ---------------------------------------------------------------------
// CurrentClassInfo
// Plain data bundle: the resolved result of "what's happening right
// now," ready to print. No logic lives here — just storage.
// ---------------------------------------------------------------------
struct CurrentClassInfo {
    bool found;          // false = nothing scheduled (free period / skipped category)
    bool cancelled;       // true = a TemporaryOverride cancelled this class today
    std::string subjectName;
    std::string venueName;
    std::string facultyName;
};

// ---------------------------------------------------------------------
// LiveClassResolver
// The actual "fetch what's happening now" logic. Takes a Section, a
// semester, a day/time/date, and a student's group — resolves it
// through that Section's Timetable, and returns a CurrentClassInfo.
// ---------------------------------------------------------------------
class LiveClassResolver {
public:
    static CurrentClassInfo resolve(Section& section, int semesterNumber,
                                     const std::string& day, int hour, int minute,
                                     const std::string& date, GroupScope studentGroup);
};

#endif // LIVE_CLOCK_H
