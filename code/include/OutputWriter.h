#ifndef OUTPUTWRITER_H
#define OUTPUTWRITER_H

#include "models/AssignmentData.h"
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <map>

/**
 * @brief Writes assignment results and risk analysis to an output CSV file.
 */
class OutputWriter {
public:
    /**
     * @brief Write the full output file.
     * @param filename       Output file path
     * @param result         Assignment results
     * @param riskAnalysis   Risk analysis K value (0 = no risk section)
     * @param atRiskReviewers Sorted list of at-risk reviewer IDs
     */
    static void write(const std::string &filename,
                      const AssignmentResult &result,
                      int riskAnalysis,
                      const std::vector<int> &atRiskReviewers);
};

#endif
