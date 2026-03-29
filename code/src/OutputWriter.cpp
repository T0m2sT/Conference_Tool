#include "OutputWriter.h"


void OutputWriter::write(const std::string &filename,
                         const AssignmentResult &result,
                         int riskAnalysis,
                         const std::vector<int> &atRiskReviewers)
{
    std::ofstream file(filename);
    if (!file.is_open()) return;

    writeAssignments(file, result);
    writeMissingReviews(file, result);
    writeRiskAnalysis(file, riskAnalysis, atRiskReviewers);

    file.close();
}


void OutputWriter::writeAssignments(std::ofstream &file,
                                    const AssignmentResult &result)
{
    // --- Assignment by submission ---
    file << "#SubmissionId,ReviewerId,Match\n";
    for (const auto &a : result.assignments) {
        file << a.submissionId << ", "
             << a.reviewerId << ", "
             << a.matchedDomain << "\n";
    }

    // --- Assignment by reviewer ---
    std::vector<Assignment> byReviewer = result.assignments;

    std::sort(byReviewer.begin(), byReviewer.end(),
              [](const Assignment &a, const Assignment &b) {
                  return a.reviewerId < b.reviewerId ||
                         (a.reviewerId == b.reviewerId &&
                          a.submissionId < b.submissionId);
              });

    file << "#ReviewerId,SubmissionId,Match\n";
    for (const auto &a : byReviewer) {
        file << a.reviewerId << ", "
             << a.submissionId << ", "
             << a.matchedDomain << "\n";
    }

    // --- Total ---
    file << "#Total: " << result.totalAssignments << "\n";
}


void OutputWriter::writeMissingReviews(std::ofstream &file,
                                       const AssignmentResult &result)
{
    if (result.missingReviews.empty()) return;

    file << "#SubmissionId,Domain,MissingReviews\n";
    for (const auto &m : result.missingReviews) {
        file << m.submissionId << ", "
             << m.domain << ", "
             << m.missingCount << "\n";
    }
}


void OutputWriter::writeRiskAnalysis(std::ofstream &file,
                                     int riskAnalysis,
                                     const std::vector<int> &atRiskReviewers)
{
    if (riskAnalysis <= 0) return;

    file << "#Risk Analysis: " << riskAnalysis << "\n";

    for (size_t i = 0; i < atRiskReviewers.size(); i++) {
        if (i > 0) file << ", ";
        file << atRiskReviewers[i];
    }

    file << "\n";
}