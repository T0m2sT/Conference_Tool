#ifndef INPUTVALIDATOR_H
#define INPUTVALIDATOR_H

/**
 * @file InputValidator.h
 * @brief Validates parsed conference data for consistency and correctness.
 *
 * Checks submissions, reviewers, parameters, and control settings for
 * issues such as duplicate IDs, invalid domains, out-of-range parameters,
 * shifted CSV fields, and feasibility constraints.
 */

#include "models/ConferenceData.h"
#include <vector>
#include <string>

/**
 * @brief Validates parsed input data and collects all errors as a summary.
 *
 * Separates validation logic from parsing, ensuring the parser only
 * handles file reading and tokenization while this class handles
 * all consistency and correctness checks.
 */
class InputValidator {
public:
    /**
     * @brief Runs all validation checks on the parsed data.
     *
     * @param submissions Parsed submissions.
     * @param reviewers Parsed reviewers.
     * @param params Parsed parameters.
     * @param control Parsed control settings.
     * @return True if all checks pass, false if any errors were found.
     *
     * **Time Complexity:** O(|S| · |R|) dominated by domain coverage check.
     */
    bool validate(const std::vector<Submission> &submissions,
                  const std::vector<Reviewer> &reviewers,
                  const Parameters &params,
                  const Control &control);

    /**
     * @brief Returns the list of errors found during the last validation.
     * @return Const reference to the error list.
     */
    const std::vector<std::string> &getErrors() const;

    /**
     * @brief Prints all collected errors to stderr as a summary.
     */
    void printErrors() const;

private:
    std::vector<std::string> errors_;

    void validateSubmissions(const std::vector<Submission> &submissions);
    void validateReviewers(const std::vector<Reviewer> &reviewers);
    void validateFieldFormats(const std::vector<Submission> &submissions,
                              const std::vector<Reviewer> &reviewers);
    void validateParameters(const Parameters &params);
    void validateControl(const Control &control);
    void validateCrossConstraints(const std::vector<Submission> &submissions,
                                  const std::vector<Reviewer> &reviewers,
                                  const Parameters &params,
                                  const Control &control);
};

#endif /* INPUTVALIDATOR_H */
