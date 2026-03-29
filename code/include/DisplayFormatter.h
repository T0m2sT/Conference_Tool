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
};

#endif
