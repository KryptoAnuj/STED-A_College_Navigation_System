#ifndef USER_H
#define USER_H

#include <string>
#include "Section.h" // Section, GroupScope
#include "Core.h"    // Venue

// ---------------------------------------------------------------------
// User (abstract base)
// Shared identity for anyone who logs in: a name and a login ID.
// Never created directly — only Admin and Student are.
// ---------------------------------------------------------------------
class User {
protected:
    std::string name;
    std::string userId;

public:
    User(std::string name, std::string userId);
    virtual ~User();

    std::string getName() const;
    std::string getUserId() const;

    // Pure virtual: forces Admin and Student to each say what they are.
    // Calling getRole() on a User pointer runs the correct derived
    // version automatically — the same polymorphism pattern as
    // Subject::describe().
    virtual std::string getRole() const = 0;
};

// ---------------------------------------------------------------------
// Admin
// Has write access: create/edit/delete entries, add temporary
// overrides. Every mutating call here forwards to the relevant
// Section's Timetable, which logs it to that Timetable's ChangeLog
// automatically.
// ---------------------------------------------------------------------
class Admin : public User {
public:
    Admin(std::string name, std::string userId);

    std::string getRole() const override;

    bool createTimetableForSection(Section& section, int semesterNumber);

    bool addClassEntry(Section& section, int semesterNumber,
                        const ScheduleEntry& entry, const std::string& timestamp);

    bool editClassVenue(Section& section, int semesterNumber,
                         const TimeSlot& slot, GroupScope scope,
                         const Venue& newVenue, const std::string& timestamp);

    bool deleteClassEntry(Section& section, int semesterNumber,
                           const TimeSlot& slot, GroupScope scope,
                           const std::string& timestamp);

    bool addTemporaryOverride(Section& section, int semesterNumber,
                               const TemporaryOverride& override,
                               const std::string& timestamp);
};

// ---------------------------------------------------------------------
// Student
// Read-only: knows their Section and roll number, and can work out
// their own G1/G2 group from that. Never touches the ChangeLog.
// ---------------------------------------------------------------------
class Student : public User {
private:
    Section* section; // which section this student belongs to
    int rollNumber;

public:
    Student(std::string name, std::string userId, Section* section, int rollNumber);

    std::string getRole() const override;

    Section* getSection() const;
    int getRollNumber() const;
    GroupScope getGroup() const; // derived from rollNumber via Section::resolveGroup
};

#endif // USER_H
