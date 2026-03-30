#include "parser/InputValidator.h"
#include <iostream>
#include <unordered_set>

const std::vector<std::string> &InputValidator::getErrors() const {
    return errors_;
}

void InputValidator::printErrors() const {
    if (errors_.empty()) return;

    std::cerr << "Error: Input file rejected — " << errors_.size()
              << " issue(s) found:" << std::endl;
    for (const auto &err : errors_) {
        std::cerr << "  - " << err << std::endl;
    }
}

bool InputValidator::validate(const std::vector<Submission> &submissions,
                              const std::vector<Reviewer> &reviewers,
                              const Parameters &params,
                              const Control &control) {
    errors_.clear();

    if (submissions.empty()) {
        errors_.push_back("No submissions found in the input file.");
    }
    if (reviewers.empty()) {
        errors_.push_back("No reviewers found in the input file.");
    }

    validateSubmissions(submissions);
    validateReviewers(reviewers);
    validateFieldFormats(submissions, reviewers);
    validateParameters(params);
    validateControl(control);
    validateCrossConstraints(submissions, reviewers, params, control);

    return errors_.empty();
}

void InputValidator::validateSubmissions(const std::vector<Submission> &submissions) {
    std::unordered_set<int> seenIds;

    for (const auto &s : submissions) {
        if (s.id <= 0) {
            errors_.push_back("Submission ID must be positive (found " +
                              std::to_string(s.id) + ").");
        }

        if (s.title.empty()) {
            errors_.push_back("Submission " + std::to_string(s.id) +
                              " has an empty title.");
        }
        if (s.authors.empty()) {
            errors_.push_back("Submission " + std::to_string(s.id) +
                              " has no authors.");
        }
        if (s.email.empty()) {
            errors_.push_back("Submission " + std::to_string(s.id) +
                              " has no contact email.");
        }

        if (s.primaryDomain <= 0) {
            errors_.push_back("Submission " + std::to_string(s.id) +
                              " has invalid primary domain (" +
                              std::to_string(s.primaryDomain) + ").");
        }
        if (s.secondaryDomain != -1 && s.secondaryDomain <= 0) {
            errors_.push_back("Submission " + std::to_string(s.id) +
                              " has invalid secondary domain (" +
                              std::to_string(s.secondaryDomain) + ").");
        }
        if (s.secondaryDomain != -1 && s.secondaryDomain == s.primaryDomain) {
            errors_.push_back("Submission " + std::to_string(s.id) +
                              " has identical primary and secondary domains (" +
                              std::to_string(s.primaryDomain) + ").");
        }

        if (s.id > 0 && !seenIds.insert(s.id).second) {
            errors_.push_back("Duplicate submission ID " +
                              std::to_string(s.id) + ".");
        }
    }
}

void InputValidator::validateReviewers(const std::vector<Reviewer> &reviewers) {
    std::unordered_set<int> seenIds;

    for (const auto &r : reviewers) {
        if (r.id <= 0) {
            errors_.push_back("Reviewer ID must be positive (found " +
                              std::to_string(r.id) + ").");
        }

        if (r.name.empty()) {
            errors_.push_back("Reviewer " + std::to_string(r.id) +
                              " has no name.");
        }
        if (r.email.empty()) {
            errors_.push_back("Reviewer " + std::to_string(r.id) +
                              " has no contact email.");
        }

        if (r.primaryExpertise <= 0) {
            errors_.push_back("Reviewer " + std::to_string(r.id) +
                              " has invalid primary expertise (" +
                              std::to_string(r.primaryExpertise) + ").");
        }
        if (r.secondaryExpertise != -1 && r.secondaryExpertise <= 0) {
            errors_.push_back("Reviewer " + std::to_string(r.id) +
                              " has invalid secondary expertise (" +
                              std::to_string(r.secondaryExpertise) + ").");
        }
        if (r.secondaryExpertise != -1 && r.secondaryExpertise == r.primaryExpertise) {
            errors_.push_back("Reviewer " + std::to_string(r.id) +
                              " has identical primary and secondary expertise (" +
                              std::to_string(r.primaryExpertise) + ").");
        }

        if (r.id > 0 && !seenIds.insert(r.id).second) {
            errors_.push_back("Duplicate reviewer ID " +
                              std::to_string(r.id) + ".");
        }
    }
}

void InputValidator::validateFieldFormats(const std::vector<Submission> &submissions,
                                          const std::vector<Reviewer> &reviewers) {
    for (const auto &s : submissions) {
        if (!s.email.empty() && s.email.find('@') == std::string::npos) {
            errors_.push_back("Submission " + std::to_string(s.id) +
                              " has invalid email '" + s.email +
                              "' (missing '@'). Possible shifted CSV fields.");
        }
        if (!s.title.empty() && s.title.find('@') != std::string::npos) {
            errors_.push_back("Submission " + std::to_string(s.id) +
                              " title looks like an email '" + s.title +
                              "'. Possible shifted CSV fields.");
        }
        if (!s.authors.empty() && s.authors.find('@') != std::string::npos) {
            errors_.push_back("Submission " + std::to_string(s.id) +
                              " authors field looks like an email '" + s.authors +
                              "'. Possible shifted CSV fields.");
        }
    }

    for (const auto &r : reviewers) {
        if (!r.email.empty() && r.email.find('@') == std::string::npos) {
            errors_.push_back("Reviewer " + std::to_string(r.id) +
                              " has invalid email '" + r.email +
                              "' (missing '@'). Possible shifted CSV fields.");
        }
        if (!r.name.empty() && r.name.find('@') != std::string::npos) {
            errors_.push_back("Reviewer " + std::to_string(r.id) +
                              " name looks like an email '" + r.name +
                              "'. Possible shifted CSV fields.");
        }
    }
}

void InputValidator::validateParameters(const Parameters &params) {
    if (params.minReviewsPerSubmission <= 0) {
        errors_.push_back("MinReviewsPerSubmission must be positive (found " +
                          std::to_string(params.minReviewsPerSubmission) + ").");
    }
    if (params.maxReviewsPerReviewer <= 0) {
        errors_.push_back("MaxReviewsPerReviewer must be positive (found " +
                          std::to_string(params.maxReviewsPerReviewer) + ").");
    }
}

void InputValidator::validateControl(const Control &control) {
    if (control.generateAssignments < 0 || control.generateAssignments > 3) {
        errors_.push_back("GenerateAssignments must be 0-3 (found " +
                          std::to_string(control.generateAssignments) + ").");
    }
    if (control.riskAnalysis < 0) {
        errors_.push_back("RiskAnalysis must be non-negative (found " +
                          std::to_string(control.riskAnalysis) + ").");
    }
    if (control.generateAssignments != 0 && control.outputFileName.empty()) {
        errors_.push_back("OutputFileName is empty but GenerateAssignments requires output.");
    }
}

void InputValidator::validateCrossConstraints(const std::vector<Submission> &submissions,
                                               const std::vector<Reviewer> &reviewers,
                                               const Parameters &params,
                                               const Control &control) {
    if (submissions.empty() || reviewers.empty()) return;

    int totalRequired = (int)submissions.size() * params.minReviewsPerSubmission;
    int totalCapacity = (int)reviewers.size() * params.maxReviewsPerReviewer;

    if (totalCapacity < totalRequired) {
        errors_.push_back("Insufficient reviewer capacity:");
        errors_.push_back(std::to_string(totalRequired) +
                          " reviews needed, " +
                          std::to_string(totalCapacity) + " available" + " (" + std::to_string(reviewers.size()) +
                          " reviewers x " + std::to_string(params.maxReviewsPerReviewer) + " max each)");
    }

    int mode = control.generateAssignments;
    if (mode == 0) mode = 1;

    for (const auto &s : submissions) {
        bool hasMatch = false;

        for (const auto &r : reviewers) {
            if (s.primaryDomain == r.primaryExpertise) { hasMatch = true; break; }

            if (mode >= 2 && s.secondaryDomain != -1 &&
                s.secondaryDomain == r.primaryExpertise) { hasMatch = true; break; }

            if (mode >= 3) {
                if (r.secondaryExpertise != -1 &&
                    s.primaryDomain == r.secondaryExpertise) { hasMatch = true; break; }
                if (s.secondaryDomain != -1 && r.secondaryExpertise != -1 &&
                    s.secondaryDomain == r.secondaryExpertise) { hasMatch = true; break; }
            }
        }

        if (!hasMatch) {
            errors_.push_back("Submission " + std::to_string(s.id) +
                              " (domain " + std::to_string(s.primaryDomain) +
                              ") has no matching reviewer in mode " +
                              std::to_string(mode) + ".");
        }
    }
}
