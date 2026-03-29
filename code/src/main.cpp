#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <termios.h>
#include <unistd.h>
#include "parser/CSVParser.h"
#include "models/ConferenceData.h"
#include "FlowNetwork.h"
#include "OutputWriter.h"
#include "DisplayFormatter.h"
#include "Menu.h"

/**
 * @brief Runs the assignment algorithm and returns formatted display lines.
 *
 * Bridges the algorithm (FlowNetwork) and the UI (Menu) without either
 * knowing about the other. Also writes the output file as a side effect.
 */
std::vector<std::string> runAssignment(const std::vector<Submission> &submissions,
                                       const std::vector<Reviewer> &reviewers,
                                       const Parameters &params,
                                       const Control &control) {
    if (submissions.empty() || reviewers.empty()) {
        return {"No data loaded."};
    }

    FlowNetwork flowNet;
    int mode = control.generateAssignments;
    if (mode == 0) mode = 1;

    AssignmentResult result = flowNet.buildAndSolve(submissions, reviewers, params, mode);

    std::vector<int> atRisk;
    if (control.riskAnalysis >= 1) {
        atRisk = flowNet.riskAnalysisK1(submissions, reviewers, params, mode);
    }

    if (control.generateAssignments != 0) {
        OutputWriter::write(control.outputFileName, result, control.riskAnalysis, atRisk);
    }

    return DisplayFormatter::formatAssignmentResult(result, control, atRisk);
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

    control.outputFileName = outputFile;

    FlowNetwork flowNet;
    int mode = control.generateAssignments;
    if (mode == 0) mode = 1;

    AssignmentResult result = flowNet.buildAndSolve(submissions, reviewers, params, mode);

    std::vector<int> atRisk;
    if (control.riskAnalysis >= 1) {
        atRisk = flowNet.riskAnalysisK1(submissions, reviewers, params, mode);
    }

    OutputWriter::write(outputFile, result, control.riskAnalysis, atRisk);
    return 0;
}

struct termios g_originalTermios;

void restoreTerminal(int) {
    tcsetattr(STDIN_FILENO, TCSANOW, &g_originalTermios);
    std::cout << "\033[?25h" << std::flush;
    std::exit(0);
}

int main(int argc, char *argv[]) {
    tcgetattr(STDIN_FILENO, &g_originalTermios);
    std::signal(SIGINT, restoreTerminal);

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
    std::string loadedFile;
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
                    loadedFile = filename;
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
                menu.displayInBox("Submissions", DisplayFormatter::formatSubmissions(submissions));
                break;
            case 2:
                menu.displayInBox("Reviewers", DisplayFormatter::formatReviewers(reviewers));
                break;
            case 3: {
                auto settingsLines = DisplayFormatter::formatSettings(params, control);
                if (!loaded)
                    settingsLines.insert(settingsLines.begin(), "No file loaded. Showing default values.");
                else
                    settingsLines.insert(settingsLines.begin(), "Loaded from: " + loadedFile);
                menu.displayInBox("Parameters & Control", settingsLines);
                break;
            }
            case 4:
                menu.displayInBox("Assignment Result", runAssignment(submissions, reviewers, params, control));
                break;
            case 5:
                std::cout << "\033[?25h" << Menu::CLEAR_SCREEN;
                return 0;
        }
    }

    return 0;
}
