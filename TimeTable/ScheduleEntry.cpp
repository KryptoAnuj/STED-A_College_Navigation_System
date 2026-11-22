#include "ScheduleEntry.h"

// =======================================================================
// ScheduleEntry
// =======================================================================

ScheduleEntry::ScheduleEntry(TimeSlot slot, std::shared_ptr<Subject> subject,
                              Venue venue, GroupScope groupScope)
    : slot(slot), subject(subject), venue(venue), groupScope(groupScope) {}

TimeSlot ScheduleEntry::getTimeSlot() const { return slot; }
std::shared_ptr<Subject> ScheduleEntry::getSubject() const { return subject; }
Venue ScheduleEntry::getVenue() const { return venue; }
GroupScope ScheduleEntry::getGroupScope() const { return groupScope; }

void ScheduleEntry::setVenue(const Venue& newVenue) { venue = newVenue; }
void ScheduleEntry::setSubject(std::shared_ptr<Subject> newSubject) { subject = newSubject; }

bool ScheduleEntry::clashesWith(const ScheduleEntry& other) const {
    // First condition: do the time slots overlap at all?
    if (!slot.overlaps(other.slot)) {
        return false;
    }

    // Second condition: do the group scopes overlap?
    // BOTH (or NONE, which behaves the same) overlaps with everything.
    // G1 only overlaps with G1 or BOTH/NONE.
    // G2 only overlaps with G2 or BOTH/NONE.
    bool thisIsUniversal = (groupScope == GroupScope::BOTH || groupScope == GroupScope::NONE);
    bool otherIsUniversal = (other.groupScope == GroupScope::BOTH || other.groupScope == GroupScope::NONE);

    if (thisIsUniversal || otherIsUniversal) {
        return true; // one of them applies to everyone -> real clash
    }

    // Neither is universal, so both are specifically G1 or G2.
    // Only a genuine clash if they're the SAME specific group.
    return groupScope == other.groupScope;
}

// =======================================================================
// ChangeRecord
// =======================================================================

ChangeRecord::ChangeRecord(std::string fieldChanged, std::string oldValue,
                            std::string newValue, std::string changedByAdminId,
                            std::string timestamp)
    : fieldChanged(fieldChanged), oldValue(oldValue), newValue(newValue),
      changedByAdminId(changedByAdminId), timestamp(timestamp) {}

std::string ChangeRecord::getFieldChanged() const { return fieldChanged; }
std::string ChangeRecord::getOldValue() const { return oldValue; }
std::string ChangeRecord::getNewValue() const { return newValue; }
std::string ChangeRecord::getChangedByAdminId() const { return changedByAdminId; }
std::string ChangeRecord::getTimestamp() const { return timestamp; }

std::string ChangeRecord::describe() const {
    return "[" + timestamp + "] " + changedByAdminId + " changed " +
           fieldChanged + " from '" + oldValue + "' to '" + newValue + "'";
}

// =======================================================================
// TemporaryOverride
// =======================================================================

TemporaryOverride::TemporaryOverride(std::string date, TimeSlot affectedSlot,
                                      bool isCancelled, Venue newVenue,
                                      std::string reason)
    : date(date), affectedSlot(affectedSlot), isCancelled(isCancelled),
      newVenue(newVenue), reason(reason) {}

std::string TemporaryOverride::getDate() const { return date; }
TimeSlot TemporaryOverride::getAffectedSlot() const { return affectedSlot; }
bool TemporaryOverride::getIsCancelled() const { return isCancelled; }
Venue TemporaryOverride::getNewVenue() const { return newVenue; }
std::string TemporaryOverride::getReason() const { return reason; }

bool TemporaryOverride::appliesTo(const std::string& queryDate, const TimeSlot& querySlot) const {
    // Must match both the exact date AND the exact time slot — a
    // temporary override is scoped to one specific class occurrence.
    return date == queryDate && affectedSlot == querySlot;
}
