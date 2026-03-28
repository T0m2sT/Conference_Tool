#include <iostream>
#include <string>
#include <vector>
#include "parser/CSVParser.h"
#include "models/ConferenceData.h"
#include "FlowNetwork.h"
#include "OutputWriter.h"

void showMenu() {
    std::cout << "\n=== Conference Review Assignment Tool ===\n";
    std::cout << "1. Load input file\n";
    std::cout << "2. Show submissions\n";
    std::cout << "3. Show reviewers\n";
    std::cout << "4. Show parameters & control settings\n";
    std::cout << "5. Run assignment\n";
    std::cout << "6. Exit\n";
    std::cout << "Choose an option: ";
}

void showSubmissions(const std::vector<Submission> &submissions) {
    if (submissions.empty()) { std::cout << "No submissions loaded.\n"; return; }
    std::cout << "\n--- Submissions ---\n";
    for (const auto &s : submissions) {
        std::cout << "ID: " << s.id
                  << " | Title: " << s.title
                  << " | Authors: " << s.authors
                  << " | Primary: " << s.primaryDomain;
        if (s.secondaryDomain != -1) std::cout << " | Secondary: " << s.secondaryDomain;
        std::cout << "\n";
    }
}

void showReviewers(const std::vector<Reviewer> &reviewers) {
    if (reviewers.empty()) { std::cout << "No reviewers loaded.\n"; return; }
    std::cout << "\n--- Reviewers ---\n";
    for (const auto &r : reviewers) {
        std::cout << "ID: " << r.id
                  << " | Name: " << r.name
                  << " | Primary Expertise: " << r.primaryExpertise;
        if (r.secondaryExpertise != -1) std::cout << " | Secondary: " << r.secondaryExpertise;
        std::cout << "\n";
    }
}

void showSettings(const Parameters &params, const Control &control) {
    std::cout << "\n--- Parameters ---\n"
              << "MinReviewsPerSubmission: " << params.minReviewsPerSubmission << "\n"
              << "MaxReviewsPerReviewer: " << params.maxReviewsPerReviewer << "\n"
              << "PrimaryReviewerExpertise: " << params.primaryReviewerExpertise << "\n"
              << "SecondaryReviewerExpertise: " << params.secondaryReviewerExpertise << "\n"
              << "PrimarySubmissionDomain: " << params.primarySubmissionDomain << "\n"
              << "SecondarySubmissionDomain: " << params.secondarySubmissionDomain << "\n";
    std::cout << "\n--- Control ---\n"
              << "GenerateAssignments: " << control.generateAssignments << "\n"
              << "RiskAnalysis: " << control.riskAnalysis << "\n"
              << "OutputFileName: " << control.outputFileName << "\n";
}

void runAssignment(const std::vector<Submission> &submissions,
                   const std::vector<Reviewer> &reviewers,
                   const Parameters &params,
                   const Control &control) {
    if (submissions.empty() || reviewers.empty()) {
        std::cout << "Please load an input file first.\n";
        return;
    }

    FlowNetwork flowNet;
    int mode = control.generateAssignments;
    if (mode == 0) mode = 1; // mode 0 still runs assignment, just doesn't report

    AssignmentResult result = flowNet.buildAndSolve(submissions, reviewers, params, mode);

    // Risk analysis
    std::vector<int> atRisk;
    if (control.riskAnalysis == 1) {
        atRisk = flowNet.riskAnalysisK1(submissions, reviewers, params, mode);
    }

    // Print summary to console
    std::cout << "\n--- Assignment Result ---\n";
    std::cout << "Total assignments: " << result.totalAssignments << "\n";
    if (result.fullySatisfied) {
        std::cout << "All submissions fully assigned.\n";
    } else {
        std::cout << "Some submissions could not be fully assigned:\n";
        for (const auto &m : result.missingReviews) {
            std::cout << "  Submission " << m.submissionId
                      << " (domain " << m.domain << "): missing "
                      << m.missingCount << " reviews\n";
        }
    }

    if (control.riskAnalysis == 1) {
        std::cout << "Risk Analysis (K=1): ";
        if (atRisk.empty()) {
            std::cout << "No at-risk reviewers.\n";
        } else {
            for (size_t i = 0; i < atRisk.size(); i++) {
                if (i > 0) std::cout << ", ";
                std::cout << atRisk[i];
            }
            std::cout << "\n";
        }
    }

    // Write output file
    if (control.generateAssignments != 0) {
        OutputWriter::write(control.outputFileName, result, control.riskAnalysis, atRisk);
        std::cout << "Output written to: " << control.outputFileName << "\n";
    }
}

int runBatchMode(const std::string &inputFile, const std::string &outputFile) {
    CSVParser parser;
    std::vector<Submission> submissions;
    std::vector<Reviewer> reviewers;
    Parameters params;
    Control control;

    if (!parser.parseFile(inputFile, submissions, reviewers, params, control)) {
        std::cerr << "Error: Failed to parse input file: " << inputFile << "\n";
        return 1;
    }

    // Override output filename from command line
    control.outputFileName = outputFile;

    FlowNetwork flowNet;
    int mode = control.generateAssignments;
    if (mode == 0) mode = 1;

    AssignmentResult result = flowNet.buildAndSolve(submissions, reviewers, params, mode);

    std::vector<int> atRisk;
    if (control.riskAnalysis == 1) {
        atRisk = flowNet.riskAnalysisK1(submissions, reviewers, params, mode);
    }

    OutputWriter::write(outputFile, result, control.riskAnalysis, atRisk);
    return 0;
}

int main(int argc, char *argv[]) {
    // Batch mode: ./da_tool -b input.csv output.csv
    if (argc == 4 && std::string(argv[1]) == "-b") {
        return runBatchMode(argv[2], argv[3]);
    }

    // Interactive mode
    std::vector<Submission> submissions;
    std::vector<Reviewer> reviewers;
    Parameters params;
    Control control;
    CSVParser parser;
    bool loaded = false;

    int choice;
    while (true) {
        showMenu();
        if (!(std::cin >> choice)) break;

        switch (choice) {
            case 1: {
                std::string defaultpath = "dataset/input/";
                std::string defaultextension = ".csv";
                std::string filename;
                std::cout << "Enter input file name: ";
                std::cin >> filename;
                submissions.clear();
                reviewers.clear();
                params = Parameters();
                control = Control();
                filename = defaultpath + filename + defaultextension;
                if (parser.parseFile(filename, submissions, reviewers, params, control)) {
                    loaded = true;
                    std::cout << "Loaded " << submissions.size() << " submissions and "
                              << reviewers.size() << " reviewers.\n";
                } else {
                    std::cerr << "Error: Failed to parse file.\n";
                    // Allows the user to view how file's path is interpreted in case of error, for correction in next menu interaction without leaving the session.
                    std::cout << "\nConfirm path wanted: \"" << filename << "\"\n"; 
                }
                break;
            }
            case 2: showSubmissions(submissions); break;
            case 3: showReviewers(reviewers); break;
            case 4: showSettings(params, control); break;
            case 5: runAssignment(submissions, reviewers, params, control); break;
            case 6: return 0;
            default: std::cout << "Invalid option.\n";
        }
    }

    return 0;
}
