#ifndef EMBEDDED_DATA_H
#define EMBEDDED_DATA_H

#include <string>

// Return the build-embedded CSV resource identified by its repository path.
std::string embeddedCsv(const std::string& path);

#endif
