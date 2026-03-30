#ifndef DISPLAYFORMATTER_H
#define DISPLAYFORMATTER_H

#include <string>
#include <vector>
#include "models/ConferenceData.h"
#include "models/AssignmentData.h"

/**
 * @brief Converts domain data into display-ready string lines.
 *
 * Responsible for formatting submissions, reviewers, parameters,
 * and assignment results into vectors of strings that the Menu can render.
 * Also provides text wrapping utilities for display formatting.
 */
class DisplayFormatter {
public:
    static std::vector<std::string> formatSubmissions(const std::vector<Submission> &submissions);
    static std::vector<std::string> formatReviewers(const std::vector<Reviewer> &reviewers);
    static std::vector<std::string> formatSettings(const Parameters &params, const Control &control);
    static std::vector<std::string> formatAssignmentResult(
        const AssignmentResult &result,
        const Control &control,
        const std::vector<int> &atRiskReviewers);

    /**
     * @brief Wraps a string into multiple lines that fit within a maximum width.
     *
     * If the prefix starts with "!!", continuation lines also get "!!"
     * so that displayInBox renders them all in red.
     *
     * @param prefix The prefix for the first line (e.g. "- " or "!!")
     * @param text The text to wrap
     * @param maxWidth Maximum width including the prefix
     * @return Vector of wrapped lines
     */
    static std::vector<std::string> wrapLine(const std::string &prefix, const std::string &text, int maxWidth);
};

#endif
