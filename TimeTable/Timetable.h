#ifndef TIMETABLE_H
#define TIMETABLE_H

#include <string>
#include <vector>
#include <memory>
#include "ScheduleEntry.h" // ScheduleEntry, ChangeRecord, TemporaryOverride, GroupScope

// ---------------------------------------------------------------------
// ChangeLog
// Owned by a Timetable. Just a growing list of ChangeRecord entries —
// the admin audit trail for one section's one-semester timetable.
// Records both permanent edits and temporary overrides.
// ---------------------------------------------------------------------
class ChangeLog {
private:
    std::vector<ChangeRecord> records;

public:
    void addRecord(const ChangeRecord& record);
    const std::vector<ChangeRecord>& getAllRecords() const;
    void printAll() const; // quick terminal dump, useful for Phase 2 demo
};

// ---------------------------------------------------------------------
// Timetable
// Holds ONE section's full schedule for ONE semester: the base
// recurring ScheduleEntry list, a ChangeLog, and a list of
// TemporaryOverrides. This is the class that actually answers
// "what's scheduled, and what's really happening right now."
// ---------------------------------------------------------------------
class Timetable {
private:
    int semesterNumber;
    std::vector<ScheduleEntry> entries;
    std::vector<TemporaryOverride> overrides;
    ChangeLog changeLog;

public:
    Timetable(int semesterNumber);

    int getSemesterNumber() const;

    // --- Base schedule management (permanent changes) ---
    // Returns false (and adds nothing) if the new entry clashes with
    // an existing one, so callers must check the return value.
    bool addEntry(const ScheduleEntry& entry, const std::string& adminId,
                  const std::string& timestamp);

    bool editEntryVenue(const TimeSlot& slot, GroupScope scope,
                        const Venue& newVenue, const std::string& adminId,
                        const std::string& timestamp);

    bool deleteEntry(const TimeSlot& slot, GroupScope scope,
                      const std::string& adminId, const std::string& timestamp);

    // --- Temporary overrides (one-day exceptions) ---
    void addOverride(const TemporaryOverride& override,
                      const std::string& adminId, const std::string& timestamp);

    // --- Queries ---
    std::vector<ScheduleEntry> getEntriesForDay(const std::string& day) const;

    // Checks overrides first (for the given date), falls back to the
    // base entry if no override applies. Returns true if something was
    // found and fills outVenue/outSubject/outCancelled accordingly.
    bool resolveClassAt(const std::string& day, int hour, int minute,
                         const std::string& date, GroupScope studentGroup,
                         std::shared_ptr<Subject>& outSubject, Venue& outVenue,
                         bool& outCancelled) const;

    bool detectClash(const ScheduleEntry& newEntry) const;

    const ChangeLog& getChangeLog() const;
};

#endif // TIMETABLE_H
