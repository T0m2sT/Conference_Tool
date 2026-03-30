#ifndef BATCHPROCESSOR_H
#define BATCHPROCESSOR_H

#include <string>
#include <vector>

/**
 * @brief Handles batch processing of CSV files.
 *
 * Provides functionality for processing single files in batch mode
 * and running all datasets in a directory.
 */
class BatchProcessor {
public:
    /**
     * @brief Processes a single CSV file in batch mode.
     *
     * Parses the file, validates it, runs the assignment algorithm,
     * and writes the output file. Prints errors to stderr.
     *
     * @param inputFile Path to input CSV file
     * @param outputFile Path to output CSV file
     * @return 0 on success, 1 on error
     */
    static int runBatchMode(const std::string &inputFile, const std::string &outputFile);

    /**
     * @brief Runs all CSV files in dataset/input/ and collects summary lines.
     *
     * @return Vector of strings summarizing the batch run, formatted for Menu display
     */
    static std::vector<std::string> runAllDatasets();
};

#endif
