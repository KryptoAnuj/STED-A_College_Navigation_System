#include "Timetable.h"
#include <iostream>

// =======================================================================
// ChangeLog
// =======================================================================

void ChangeLog::addRecord(const ChangeRecord& record) {
    records.push_back(record);
}

const std::vector<ChangeRecord>& ChangeLog::getAllRecords() const {
    return records;
}

void ChangeLog::printAll() const {
    for (const auto& record : records) {
        std::cout << record.describe() << std::endl;
    }
}

// =======================================================================
// Timetable — helper (not declared in the header, only used internally)
// =======================================================================

// True if the given hour:minute falls inside this slot's range, on the
// same day. Used to find "which entry covers this exact moment," which
// is a different question from overlaps() (which compares two ranges).
static bool slotCovers(const TimeSlot& slot, const std::string& day,
                        int hour, int minute) {
    if (slot.getDay() != day) return false;

    int queryMinutes = hour * 60 + minute;
    int startMinutes = slot.getStartHour() * 60 + slot.getStartMinute();
    int endMinutes = slot.getEndHour() * 60 + slot.getEndMinute();

    return queryMinutes >= startMinutes && queryMinutes < endMinutes;
}

// True if an entry's group scope applies to a given student's group.
// BOTH/NONE applies to everyone; G1/G2 only applies to a matching
// student group.
static bool scopeAppliesToStudent(GroupScope entryScope, GroupScope studentGroup) {
    if (entryScope == GroupScope::BOTH || entryScope == GroupScope::NONE) {
        return true;
    }
    return entryScope == studentGroup;
}

// =======================================================================
// Timetable
// =======================================================================

Timetable::Timetable(int semesterNumber) : semesterNumber(semesterNumber) {}

int Timetable::getSemesterNumber() const { return semesterNumber; }

bool Timetable::addEntry(const ScheduleEntry& entry, const std::string& adminId,
                          const std::string& timestamp) {
    if (detectClash(entry)) {
        return false; // reject — caller should tell the admin why
    }
    entries.push_back(entry);
    changeLog.addRecord(ChangeRecord("AddEntry", "(none)",
                         entry.getSubject()->getSubjectName(), adminId, timestamp));
    return true;
}

bool Timetable::editEntryVenue(const TimeSlot& slot, GroupScope scope,
                                const Venue& newVenue, const std::string& adminId,
                                const std::string& timestamp) {
    for (auto& entry : entries) {
        if (entry.getTimeSlot() == slot && entry.getGroupScope() == scope) {
            std::string oldVenueName = entry.getVenue().getName();
            entry.setVenue(newVenue);
            changeLog.addRecord(ChangeRecord("Venue", oldVenueName,
                                 newVenue.getName(), adminId, timestamp));
            return true;
        }
    }
    return false; // no matching entry found
}

bool Timetable::deleteEntry(const TimeSlot& slot, GroupScope scope,
                             const std::string& adminId, const std::string& timestamp) {
    for (size_t i = 0; i < entries.size(); i++) {
        if (entries[i].getTimeSlot() == slot && entries[i].getGroupScope() == scope) {
            std::string removedName = entries[i].getSubject()->getSubjectName();
            entries.erase(entries.begin() + i);
            changeLog.addRecord(ChangeRecord("DeleteEntry", removedName,
                                 "(removed)", adminId, timestamp));
            return true;
        }
    }
    return false;
}

void Timetable::addOverride(const TemporaryOverride& override,
                             const std::string& adminId, const std::string& timestamp) {
    overrides.push_back(override);
    std::string summary = override.getIsCancelled()
        ? "Cancelled (" + override.getReason() + ")"
        : "Venue -> " + override.getNewVenue().getName() + " (" + override.getReason() + ")";
    changeLog.addRecord(ChangeRecord("TemporaryOverride", "(base entry unchanged)",
                         summary, adminId, timestamp));
}

std::vector<ScheduleEntry> Timetable::getEntriesForDay(const std::string& day) const {
    std::vector<ScheduleEntry> result;
    for (const auto& entry : entries) {
        if (entry.getTimeSlot().getDay() == day) {
            result.push_back(entry);
        }
    }
    return result;
}

bool Timetable::resolveClassAt(const std::string& day, int hour, int minute,
                                const std::string& date, GroupScope studentGroup,
                                std::shared_ptr<Subject>& outSubject, Venue& outVenue,
                                bool& outCancelled) const {
    outCancelled = false;

    // Step 1: find the base entry that covers this moment AND applies
    // to this student's group.
    for (const auto& entry : entries) {
        if (slotCovers(entry.getTimeSlot(), day, hour, minute) &&
            scopeAppliesToStudent(entry.getGroupScope(), studentGroup)) {

            // Step 2: check if a TemporaryOverride exists for today's
            // date and this exact slot — it takes priority.
            for (const auto& override : overrides) {
                if (override.appliesTo(date, entry.getTimeSlot())) {
                    if (override.getIsCancelled()) {
                        outCancelled = true;
                        outSubject = entry.getSubject();
                        outVenue = entry.getVenue();
                        return true;
                    } else {
                        outSubject = entry.getSubject();
                        outVenue = override.getNewVenue();
                        return true;
                    }
                }
            }

            // No override found — use the base entry as-is.
            outSubject = entry.getSubject();
            outVenue = entry.getVenue();
            return true;
        }
    }

    return false; // nothing scheduled at this moment (free period / skipped category)
}

bool Timetable::detectClash(const ScheduleEntry& newEntry) const {
    for (const auto& entry : entries) {
        if (entry.clashesWith(newEntry)) {
            return true;
        }
    }
    return false;
}

const ChangeLog& Timetable::getChangeLog() const { return changeLog; }
