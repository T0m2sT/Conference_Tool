#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include "parser/CSVParser.h"
#include "models/ConferenceData.h"
#include "FlowNetwork.h"
#include "OutputWriter.h"
#include "Menu.h"

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
    Menu menu;
    std::vector<Submission> submissions;
    std::vector<Reviewer> reviewers;
    Parameters params;
    Control control;
    CSVParser parser;
    bool loaded = false;
    std::vector<std::string> menuOptions = {
        "Load input file",
        "Show submissions",
        "Show reviewers",
        "Show parameters & control settings",
        "Run assignment",
        "Exit"
    };

    while (true) {
        int choice = menu.arrowMenu(menuOptions);

        switch (choice) {
            case 0: {
                std::string defaultpath = "dataset/input/";
                std::string defaultextension = ".csv";
                std::string filename = menu.promptInBox("Load Input File", "File name: ");
                submissions.clear();
                reviewers.clear();
                params = Parameters();
                control = Control();
                filename = defaultpath + filename + defaultextension;
                std::vector<std::string> lines;
                if (parser.parseFile(filename, submissions, reviewers, params, control)) {
                    loaded = true;
                    lines.push_back("Loaded " + std::to_string(submissions.size()) + " submissions");
                    lines.push_back("Loaded " + std::to_string(reviewers.size()) + " reviewers");
                } else {
                    lines.push_back("Error: Failed to parse file.");
                    lines.push_back("Path: \"" + filename + "\"");
                }
                menu.displayInBox("Load Input File", lines);
                break;
            }
            case 1:
                menu.displayInBox("Submissions", Menu::getSubmissionLines(submissions));
                break;
            case 2:
                menu.displayInBox("Reviewers", Menu::getReviewerLines(reviewers));
                break;
            case 3:
                menu.displayInBox("Parameters & Control", Menu::getSettingsLines(params, control));
                break;
            case 4:
                menu.displayInBox("Assignment Result", Menu::runAssignment(submissions, reviewers, params, control));
                break;
            case 5:
                std::cout << Menu::CLEAR_SCREEN;
                return 0;
        }
    }

    return 0;
}
