#pragma once

#ifndef CMS_STRING_UTIL_HPP
#define CMS_STRING_UTIL_HPP

#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unordered_set>

#include <boost/optional.hpp>
#include <spdlog/spdlog.h>

namespace string_util {

class StringReplacer {
public:
  StringReplacer(std::string_view needle, std::string_view replacement);
  ~StringReplacer() = default;

  // Thread-safe string replacement.
  // Returns new string with up to maxReplacements occurrences of needle
  // replaced. If maxReplacements is 0, all occurrences are replaced.
  std::string replace(std::string_view input, size_t maxReplacements = 0) const;

private:
  // Find match positions in the input string, up to maxReplacements.
  void findMatches(std::string_view input, std::vector<size_t> &positions,
                   size_t maxReplacements) const;

  // Calculate output buffer size based on matches.
  size_t calculateOutputSize(std::string_view input,
                             const std::vector<size_t> &positions) const;

  // Perform actual replacement into output buffer.
  void performReplacement(std::string_view input,
                          const std::vector<size_t> &positions,
                          char *output) const;

  const std::string needle_;
  const std::string replacement_;
};

/**
   Usage:
   string_util::StringReplacer replacer("old", "new");
   std::string result = replacer.replace("This is old text with old values");
   // result = "This is new text with new values"
 **/

StringReplacer::StringReplacer(std::string_view needle,
                               std::string_view replacement)
    : needle_(needle), replacement_(replacement) {
  assert(!needle.empty() && "Needle cannot be empty");
}

std::string StringReplacer::replace(std::string_view input,
                                    size_t maxReplacements) const {
  // Early exit for empty input or no possible matches
  if (input.empty() || input.size() < needle_.size()) {
    return std::string(input);
  }

  // Find match positions
  std::vector<size_t> positions;
  findMatches(input, positions, maxReplacements);

  // No matches found
  if (positions.empty()) {
    return std::string(input);
  }

  // Allocate exact output buffer size
  size_t outputSize = calculateOutputSize(input, positions);
  std::string result;
  result.resize(outputSize);

  // Perform replacement
  performReplacement(input, positions, result.data());
  return result;
}

void StringReplacer::findMatches(std::string_view input,
                                 std::vector<size_t> &positions,
                                 size_t maxReplacements) const {
  size_t pos = 0;
  size_t count = 0;
  while (pos <= input.size() - needle_.size()) {
    auto found = input.find(needle_, pos);
    if (found == std::string_view::npos) {
      break;
    }
    positions.push_back(found);
    pos = found + needle_.size();
    ++count;
    if (maxReplacements > 0 && count >= maxReplacements) {
      break;
    }
  }
}

size_t StringReplacer::calculateOutputSize(
    std::string_view input, const std::vector<size_t> &positions) const {
  return input.size() +
         positions.size() * (replacement_.size() - needle_.size());
}

void StringReplacer::performReplacement(std::string_view input,
                                        const std::vector<size_t> &positions,
                                        char *output) const {
  size_t inputPos = 0;
  size_t outputPos = 0;

  for (size_t matchPos : positions) {
    // Copy unchanged portion before match
    if (matchPos > inputPos) {
      std::copy(input.begin() + inputPos, input.begin() + matchPos,
                output + outputPos);
      outputPos += matchPos - inputPos;
    }

    // Copy replacement
    std::copy(replacement_.begin(), replacement_.end(), output + outputPos);
    outputPos += replacement_.size();

    inputPos = matchPos + needle_.size();
  }

  // Copy remaining input
  if (inputPos < input.size()) {
    std::copy(input.begin() + inputPos, input.end(), output + outputPos);
  }
}

/**
 * Format bsoncxx::types::b_date to human friendly
 * ex: Thu, June 12, 2025 at 10:33 AM UTC
 */
std::string timestamp(bsoncxx::types::b_date date) {
  // Convert milliseconds since epoch to time_point
  auto timePoint = std::chrono::system_clock::time_point(date.value);
  auto timeT = std::chrono::system_clock::to_time_t(timePoint);
  char buffer[40];
  std::strftime(buffer, sizeof(buffer), "%a, %B %d, %Y at %I:%M %p UTC",
                std::gmtime(&timeT));
  return buffer;
}

} // namespace string_util

#endif // CMS_STRING_UTIL_HPP
