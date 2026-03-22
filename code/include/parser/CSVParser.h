#ifndef CSVPARSER_H
#define CSVPARSER_H

#include "models/ConferenceData.h"
#include <string>
#include <vector>

class CSVParser
{
public:
    CSVParser() = default;

    /**
     * @brief Parse the CSV file and fill the given containers with data.
     * 
     * @param filename Path to the CSV file.
     * @param submissions Vector to store parsed Submission data.
     * @param reviewers Vector to store parsed Reviewer data.
     * @param params Reference to Parameters struct to populate.
     * @param control Reference to Control struct to populate.
     * @return true if file parsed successfully, false otherwise.
     */
    bool parseFile(const std::string &filename,
                   std::vector<Submission> &submissions,
                   std::vector<Reviewer> &reviewers,
                   Parameters &params,
                   Control &control);

private:
    /**
     * @brief Enum to keep track of the current section in the CSV file.
     */
    enum Section { NONE, SUBMISSIONS, REVIEWERS, PARAMETERS, CONTROL };

    /**
     * @brief Determine which section the line belongs to based on the header.
     * 
     * @param line CSV line starting with '#' to detect section.
     * @return Section enum value representing the section.
     */
    Section detectSection(const std::string &line);

    /**
     * @brief Parse a line of CSV based on the current section.
     * Handles submissions, reviewers, parameters, and control lines.
     * 
     * @param tokens Tokenized line split by commas.
     * @param section Current CSV section.
     * @param submissions Vector to store Submission data.
     * @param reviewers Vector to store Reviewer data.
     * @param params Reference to Parameters struct to populate.
     * @param control Reference to Control struct to populate.
     */
    void parseLine(const std::vector<std::string> &tokens,
                   Section section,
                   std::vector<Submission> &submissions,
                   std::vector<Reviewer> &reviewers,
                   Parameters &params,
                   Control &control);

    /**
     * @brief Helper function to trim whitespace from a string.
     */
    std::string trim(const std::string &s);

    /**
     * @brief Helper function to split a CSV line into tokens.
     */
    std::vector<std::string> splitCSVLine(const std::string &line);
};

#endif /* CSVPARSER_H */