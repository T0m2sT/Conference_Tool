#ifndef ERRORREPORTING_H
#define ERRORREPORTING_H

#include <string>
#include <vector>
#include "models/AssignmentData.h"

/**
 * @brief Handles error report generation and batch result formatting.
 *
 * Provides utilities for writing error reports to files and formatting
 * single dataset batch processing results.
 */
class ErrorReporting {
public:
    /**
     * @brief Writes an error report for one or more files.
     *
     * Each entry is a pair of {filename, list of errors}.
     * Entries are written sorted by filename.
     */
    static void writeErrorReport(const std::string &outputPath,
                                 std::vector<std::pair<std::string, std::vector<std::string>>> &fileErrors);

    /**
     * @brief Formats a one-line summary for a single dataset run.
     *
     * @param filename   Just the file name (e.g. "dataset1.csv").
     * @param result     The assignment result from the flow network.
     * @param errors     Validation errors (empty if none).
     * @param parseFailed True if the CSV could not be parsed at all.
     * @return A short summary string that fits inside the menu box (max ~60 chars).
     */
    static std::string formatBatchLine(const std::string &filename,
                                       const AssignmentResult &result,
                                       const std::vector<std::string> &errors,
                                       bool parseFailed);
};

#endif
