#include "DisplayFormatter.h"

std::vector<std::string> DisplayFormatter::formatSubmissions(const std::vector<Submission> &submissions) {
    std::vector<std::string> lines;
    if (submissions.empty()) { lines.push_back("No submissions loaded."); return lines; }
    for (const auto &s : submissions) {
        std::string line = "ID:" + std::to_string(s.id)
            + " | " + s.title
            + " | D:" + std::to_string(s.primaryDomain);
        if (s.secondaryDomain != -1) line += "/" + std::to_string(s.secondaryDomain);
        lines.push_back(line);
    }
    return lines;
}

std::vector<std::string> DisplayFormatter::formatReviewers(const std::vector<Reviewer> &reviewers) {
    std::vector<std::string> lines;
    if (reviewers.empty()) { lines.push_back("No reviewers loaded."); return lines; }
    for (const auto &r : reviewers) {
        std::string line = "ID:" + std::to_string(r.id)
            + " | " + r.name
            + " | E:" + std::to_string(r.primaryExpertise);
        if (r.secondaryExpertise != -1) line += "/" + std::to_string(r.secondaryExpertise);
        lines.push_back(line);
    }
    return lines;
}

std::vector<std::string> DisplayFormatter::formatSettings(const Parameters &params, const Control &control) {
    std::vector<std::string> lines;
    lines.push_back("--- Parameters ---");
    lines.push_back("MinReviews/Sub: " + std::to_string(params.minReviewsPerSubmission));
    lines.push_back("MaxReviews/Rev: " + std::to_string(params.maxReviewsPerReviewer));
    lines.push_back("PrimaryRevExpertise: " + std::to_string(params.primaryReviewerExpertise));
    lines.push_back("SecondRevExpertise: " + std::to_string(params.secondaryReviewerExpertise));
    lines.push_back("PrimarySubDomain: " + std::to_string(params.primarySubmissionDomain));
    lines.push_back("SecondSubDomain: " + std::to_string(params.secondarySubmissionDomain));
    lines.push_back("");
    lines.push_back("--- Control ---");
    lines.push_back("GenerateAssignments: " + std::to_string(control.generateAssignments));
    lines.push_back("RiskAnalysis: " + std::to_string(control.riskAnalysis));
    lines.push_back("Output: " + control.outputFileName);
    return lines;
}

std::vector<std::string> DisplayFormatter::formatAssignmentResult(
    const AssignmentResult &result,
    const Control &control,
    const std::vector<int> &atRiskReviewers)
{
    std::vector<std::string> lines;

    lines.push_back("Total: " + std::to_string(result.totalAssignments));
    lines.push_back("");
    if (result.fullySatisfied) {
        lines.push_back("All submissions fully assigned.");
    } else {
        lines.push_back("Missing reviews:");
        for (const auto &m : result.missingReviews) {
            lines.push_back("  Sub " + std::to_string(m.submissionId)
                + " (D:" + std::to_string(m.domain)
                + ") missing " + std::to_string(m.missingCount));
        }
    }

    if (control.riskAnalysis >= 1) {
        lines.push_back("");
        if (atRiskReviewers.empty()) {
            lines.push_back("Risk (K=1): No at-risk reviewers.");
        } else {
            std::string riskLine = "Risk (K=1): ";
            for (size_t i = 0; i < atRiskReviewers.size(); i++) {
                if (i > 0) riskLine += ", ";
                riskLine += std::to_string(atRiskReviewers[i]);
            }
            lines.push_back(riskLine);
        }
    }

    if (control.generateAssignments != 0) {
        lines.push_back("");
        lines.push_back("Output: " + control.outputFileName);
    }

    return lines;
}
