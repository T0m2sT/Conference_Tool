#include <iostream>
#include <vector>
#include "parser/CSVParser.h"
#include "models/ConferenceData.h"

int main() {

    std::vector<Submission> submissions;
    std::vector<Reviewer> reviewers;
    Parameters params;
    Control control;


    CSVParser parser;


    std::string filename = "../datasets/input/dataset1.csv";
    if (!parser.parseFile(filename, submissions, reviewers, params, control)) {
        std::cerr << "Failed to parse the file.\n";
        return 1;
    }


    std::cout << "Submissions:\n";
    for (const auto &s : submissions) {
        std::cout << "ID: " << s.id
                  << ", Title: " << s.title
                  << ", Authors: " << s.authors
                  << ", Email: " << s.email
                  << ", Primary Domain: " << s.primaryDomain
                  << ", Secondary Domain: " << s.secondaryDomain
                  << "\n";
    }


    std::cout << "\nReviewers:\n";
    for (const auto &r : reviewers) {
        std::cout << "ID: " << r.id
                  << ", Name: " << r.name
                  << ", Email: " << r.email
                  << ", Primary Expertise: " << r.primaryExpertise
                  << ", Secondary Expertise: " << r.secondaryExpertise
                  << "\n";
    }


    std::cout << "\nParameters:\n"
              << "MinReviewsPerSubmission: " << params.minReviewsPerSubmission << "\n"
              << "MaxReviewsPerReviewer: " << params.maxReviewsPerReviewer << "\n"
              << "PrimaryReviewerExpertise: " << params.primaryReviewerExpertise << "\n"
              << "SecondaryReviewerExpertise: " << params.secondaryReviewerExpertise << "\n"
              << "PrimarySubmissionDomain: " << params.primarySubmissionDomain << "\n"
              << "SecondarySubmissionDomain: " << params.secondarySubmissionDomain << "\n";


    std::cout << "\nControl:\n"
              << "GenerateAssignments: " << control.generateAssignments << "\n"
              << "RiskAnalysis: " << control.riskAnalysis << "\n"
              << "OutputFileName: " << control.outputFileName << "\n";

    return 0;
}