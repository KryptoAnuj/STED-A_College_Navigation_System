#ifndef SCHEDULE_ENTRY_H
#define SCHEDULE_ENTRY_H

#include <string>
#include <memory>   // for std::shared_ptr — see note near Subject below
#include "Core.h"   // TimeSlot, Venue, Subject

// ---------------------------------------------------------------------
// GroupScope
// Which student group(s) a given ScheduleEntry applies to.
// NONE and BOTH behave identically in practice (everyone attends) —
// NONE just represents "no G1/G2 marker was written on this cell at
// all," which we still treat as "both groups."
// ---------------------------------------------------------------------
enum class GroupScope {
    NONE,
    G1,
    G2,
    BOTH
};

// ---------------------------------------------------------------------
// ScheduleEntry
// One populated cell from a real timetable grid: a time slot, a
// subject, a venue, and which group(s) it applies to.
// ---------------------------------------------------------------------
class ScheduleEntry {
private:
    TimeSlot slot;
    std::shared_ptr<Subject> subject;
    // Subject is abstract (TheorySubject/PracticalSubject only), so we
    // can't store a plain Subject by value here — we store a pointer
    // to whichever derived object was actually created.
    // shared_ptr manages that memory automatically so we don't have to
    // manually delete it later.
    Venue venue;
    GroupScope groupScope;

public:
    ScheduleEntry(TimeSlot slot, std::shared_ptr<Subject> subject,
                  Venue venue, GroupScope groupScope = GroupScope::BOTH);

    TimeSlot getTimeSlot() const;
    std::shared_ptr<Subject> getSubject() const;
    Venue getVenue() const;
    GroupScope getGroupScope() const;

    void setVenue(const Venue& newVenue);
    void setSubject(std::shared_ptr<Subject> newSubject);

    // True only if both the time slots overlap AND the group scopes
    // overlap. This is what allows G1 and G2 to have two different
    // entries at the same time slot without it counting as a clash.
    bool clashesWith(const ScheduleEntry& other) const;
};

// ---------------------------------------------------------------------
// ChangeRecord
// One line in the admin audit trail: what changed, from what, to
// what, by whom, and when. Used for both permanent edits and
// temporary overrides.
// ---------------------------------------------------------------------
class ChangeRecord {
private:
    std::string fieldChanged;   // e.g. "Venue", "Subject"
    std::string oldValue;
    std::string newValue;
    std::string changedByAdminId;
    std::string timestamp;      // stored as plain text for Phase 2 simplicity

public:
    ChangeRecord(std::string fieldChanged, std::string oldValue,
                 std::string newValue, std::string changedByAdminId,
                 std::string timestamp);

    std::string getFieldChanged() const;
    std::string getOldValue() const;
    std::string getNewValue() const;
    std::string getChangedByAdminId() const;
    std::string getTimestamp() const;

    std::string describe() const; // human-readable one-line summary
};

// ---------------------------------------------------------------------
// TemporaryOverride
// A one-day exception to a base ScheduleEntry — e.g. faculty absent
// today, or room changed just for today. Does NOT modify the original
// ScheduleEntry; it's checked separately, for one specific date only.
// ---------------------------------------------------------------------
class TemporaryOverride {
private:
    std::string date;              // specific calendar date, e.g. "2026-10-20"
    TimeSlot affectedSlot;         // which slot this override applies to
    bool isCancelled;              // true if the class is simply cancelled
    Venue newVenue;                // used only if the venue changed (not cancelled)
    std::string reason;

public:
    TemporaryOverride(std::string date, TimeSlot affectedSlot,
                       bool isCancelled, Venue newVenue, std::string reason);

    std::string getDate() const;
    TimeSlot getAffectedSlot() const;
    bool getIsCancelled() const;
    Venue getNewVenue() const;
    std::string getReason() const;

    // True if this override applies to the given date and slot —
    // used by LiveClassResolver to check "is there an exception for
    // right now?" before falling back to the base entry.
    bool appliesTo(const std::string& queryDate, const TimeSlot& querySlot) const;
};

#endif // SCHEDULE_ENTRY_H
