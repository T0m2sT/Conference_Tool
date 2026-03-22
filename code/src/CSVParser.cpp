#include "parser/CSVParser.h"

#include <fstream>
#include <sstream>

std::string CSVParser::trim(const std::string &s) {
    std::size_t start = s.find_first_not_of(" \t\r\n");
    std::size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::vector<std::string> CSVParser::splitCSVLine(const std::string &line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;

    while (std::getline(ss, token, ',')) {
        tokens.push_back(trim(token));
    }

    return tokens;
}

CSVParser::Section CSVParser::detectSection(const std::string &line) {
    if (line == "#Submissions") return SUBMISSIONS;
    if (line == "#Reviewers") return REVIEWERS;
    if (line == "#Parameters") return PARAMETERS;
    if (line == "#Control") return CONTROL;

    return NONE;
}

void CSVParser::parseLine(const std::vector<std::string> &tokens, Section section,
                          std::vector<Submission> &submissions, std::vector<Reviewer> &reviewers,
                          Parameters &params, Control &control) {
    switch (section) {
        case SUBMISSIONS:
            if (tokens.size() >= 5) {
                Submission s;
                s.id = std::stoi(tokens[0]);
                s.title = tokens[1];
                s.authors = tokens[2];
                s.email = tokens[3];
                s.primaryDomain = std::stoi(tokens[4]);
                s.secondaryDomain = (tokens.size() > 5 && !tokens[5].empty()) ? std::stoi(tokens[5]) : -1;
                submissions.push_back(s);
            }
            break;

        case REVIEWERS:
            if (tokens.size() >= 4) {
                Reviewer r;
                r.id = std::stoi(tokens[0]);
                r.name = tokens[1];
                r.email = tokens[2];
                r.primaryExpertise = std::stoi(tokens[3]);
                r.secondaryExpertise = (tokens.size() > 4 && !tokens[4].empty()) ? std::stoi(tokens[4]) : -1;
                reviewers.push_back(r);
            }
            break;

        case PARAMETERS:
            if (tokens.size() >= 2) {
                std::string key = tokens[0];
                std::string value = tokens[1];
                if (key == "MinReviewsPerSubmission") params.minReviewsPerSubmission = std::stoi(value);
                else if (key == "MaxReviewsPerReviewer") params.maxReviewsPerReviewer = std::stoi(value);
                else if (key == "PrimaryReviewerExpertise") params.primaryReviewerExpertise = std::stoi(value);
                else if (key == "SecondaryReviewerExpertise") params.secondaryReviewerExpertise = std::stoi(value);
                else if (key == "PrimarySubmissionDomain") params.primarySubmissionDomain = std::stoi(value);
                else if (key == "SecondarySubmissionDomain") params.secondarySubmissionDomain = std::stoi(value);
            }
            break;

        case CONTROL:
            if (tokens.size() >= 2) {
                std::string key = tokens[0];
                std::string value = tokens[1];
                if (key == "GenerateAssignments") control.generateAssignments = std::stoi(value);
                else if (key == "RiskAnalysis") control.riskAnalysis = std::stoi(value);
                else if (key == "OutputFileName") control.outputFileName = value;
            }
            break;

        default:
            break;
    }
}

bool CSVParser::parseFile(const std::string &filename, std::vector<Submission> &submissions,
                          std::vector<Reviewer> &reviewers, Parameters &params, Control &control)
{
    // Open File
    std::ifstream file(filename);

    if (!file.is_open()) {
        // ERROR CANNOT OPEN FILE
        return false;
    }
    //

    Section section = NONE;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            CSVParser::Section newSection = detectSection(line);
            section = (newSection == NONE) ? section : newSection;
            continue;
        }

        std::vector<std::string> tokens = splitCSVLine(line);
        parseLine(tokens, section, submissions, reviewers, params, control);
    }

    file.close();
    return true;

    
}
