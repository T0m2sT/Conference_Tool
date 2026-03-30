#ifndef OUTPUTWRITER_H
#define OUTPUTWRITER_H

#include "models/AssignmentData.h"
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

/**
 * @brief Utility class responsible for writing output files.
 */
class OutputWriter {
public:
    /**
     * @brief Writes the complete output file.
     *
     * Delegates to writeAssignments, writeMissingReviews, and writeRiskAnalysis.
     *
     * **Time Complexity:** O(A log A)
     *   where A = number of assignments (dominated by the reviewer-perspective sort).
     *
     * @param filename Path to output file.
     * @param result Assignment result data.
     * @param riskAnalysis Risk analysis mode (0 = none).
     * @param atRiskReviewers Sorted list of at-risk reviewer IDs.
     */
    static void write(const std::string &filename,
                      const AssignmentResult &result,
                      int riskAnalysis,
                      const std::vector<int> &atRiskReviewers);

private:
    /**
     * @brief Writes assignment results from both perspectives (by-submission and by-reviewer).
     *
     * The by-submission list is already sorted from buildAndSolve.
     * The by-reviewer list is sorted here by reviewer ID, then submission ID.
     *
     * **Time Complexity:** O(A log A) for the reviewer-perspective sort, O(A) for writing.
     *
     * @param file Output file stream.
     * @param result Assignment result data.
     */
    static void writeAssignments(std::ofstream &file,
                                 const AssignmentResult &result);

    /**
     * @brief Writes missing reviews section (submissions that couldn't get enough reviewers).
     *
     * **Time Complexity:** O(M) where M = number of missing review entries.
     *
     * @param file Output file stream.
     * @param result Assignment result data.
     */
    static void writeMissingReviews(std::ofstream &file,
                                   const AssignmentResult &result);

    /**
     * @brief Writes risk analysis section.
     *
     * **Time Complexity:** O(K) where K = number of at-risk reviewers.
     *
     * @param file Output file stream.
     * @param riskAnalysis Risk analysis mode.
     * @param atRiskReviewers Sorted list of at-risk reviewer IDs.
     */
    static void writeRiskAnalysis(std::ofstream &file,
                                 int riskAnalysis,
                                 const std::vector<int> &atRiskReviewers);
};

#endif
