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
     * @brief Writes assignment results (both perspectives).
     * @param file Output file stream.
     * @param result Assignment result data.
     */
    static void writeAssignments(std::ofstream &file,
                                 const AssignmentResult &result);

    /**
     * @brief Writes missing reviews section.
     * @param file Output file stream.
     * @param result Assignment result data.
     */
    static void writeMissingReviews(std::ofstream &file,
                                   const AssignmentResult &result);

    /**
     * @brief Writes risk analysis section.
     * @param file Output file stream.
     * @param riskAnalysis Risk analysis mode.
     * @param atRiskReviewers Sorted list of at-risk reviewer IDs.
     */
    static void writeRiskAnalysis(std::ofstream &file,
                                 int riskAnalysis,
                                 const std::vector<int> &atRiskReviewers);
};

#endif