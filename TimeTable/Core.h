#ifndef CORE_H
#define CORE_H
// The three lines above are a "header guard" — they stop this file from
// being included twice by accident in the same build, which would cause
// a "class already defined" error. #ifndef = "if not defined",
// #define marks it as now defined, #endif closes the guard at the
// bottom of the file.

#include <string>

// ---------------------------------------------------------------------
// TimeSlot
// Represents one block of time on one day, e.g. MON 8:00-8:55.
// This is a small "value type" — it just holds data and knows how to
// compare itself to another TimeSlot. Every clash-detection check in
// the whole project ultimately depends on this class.
// ---------------------------------------------------------------------
class TimeSlot {
private:
    std::string day;       // e.g. "MON", "TUE"
    int startHour, startMinute;
    int endHour, endMinute;

public:
    TimeSlot(std::string day, int startHour, int startMinute,
              int endHour, int endMinute);

    std::string getDay() const;
    int getStartHour() const;
    int getStartMinute() const;
    int getEndHour() const;
    int getEndMinute() const;

    // Operator overloading: lets us write "slotA == slotB" directly
    // instead of calling a function with an awkward name.
    bool operator==(const TimeSlot& other) const;

    // True if this slot and 'other' share any time on the same day.
    // This is the method clash-detection will actually call.
    bool overlaps(const TimeSlot& other) const;
};

// ---------------------------------------------------------------------
// Venue
// A classroom/lab identifier. Deliberately simple — it doesn't know
// anything about the campus map or navigation nodes, which live in a
// separate part of the project built by a teammate.
// ---------------------------------------------------------------------
class Venue {
private:
    std::string name; // e.g. "CIVIL CR24", "BTECH BLK CR16"

public:
    Venue(std::string name = "TBD");
    std::string getName() const;
    bool operator==(const Venue& other) const;
};

// ---------------------------------------------------------------------
// Subject (abstract base class)
// Represents one course: code, name, faculty, and the semester it
// belongs to (parsed from the leading digit in the course code).
// This class can never be created directly — only its derived types
// (TheorySubject, PracticalSubject) can be instantiated. That's what
// "abstract" means here, enforced by the pure virtual describe().
// ---------------------------------------------------------------------
class Subject {
protected:
    std::string courseCode;   // e.g. "TPH101", "PCS151"
    std::string subjectName;  // e.g. "Engineering Physics"
    std::string facultyName;
    int semesterNumber;       // parsed from the course code's leading digit

public:
    Subject(std::string courseCode, std::string subjectName,
            std::string facultyName, int semesterNumber);

    virtual ~Subject(); // virtual destructor: required whenever a class
                         // has virtual functions and might be deleted
                         // through a base-class pointer.

    std::string getCourseCode() const;
    std::string getSubjectName() const;
    std::string getFacultyName() const;
    int getSemesterNumber() const;

    // "Pure virtual" (the "= 0" below) means every derived class MUST
    // provide its own version of this function. This is the
    // polymorphism showcase: calling describe() on any Subject runs
    // different code depending on its real type.
    virtual std::string describe() const = 0;
};

// ---------------------------------------------------------------------
// TheorySubject — a Subject that is a lecture-style theory class.
// ---------------------------------------------------------------------
class TheorySubject : public Subject {
public:
    TheorySubject(std::string courseCode, std::string subjectName,
                  std::string facultyName, int semesterNumber);

    std::string describe() const override;
};

// ---------------------------------------------------------------------
// PracticalSubject — a Subject that is a lab/practical session.
// ---------------------------------------------------------------------
class PracticalSubject : public Subject {
public:
    PracticalSubject(std::string courseCode, std::string subjectName,
                      std::string facultyName, int semesterNumber);

    std::string describe() const override;
};

#endif // CORE_H
