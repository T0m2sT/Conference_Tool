#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <csignal>
#include <filesystem>
#include <termios.h>
#include <unistd.h>
#include "parser/CSVParser.h"
#include "parser/InputValidator.h"
#include "models/ConferenceData.h"
#include "FlowNetwork.h"
#include "OutputWriter.h"
#include "DisplayFormatter.h"
#include "Menu.h"
#include "ErrorReporting.h"
#include "BatchProcessor.h"

namespace fs = std::filesystem;

static const int MAX_LINE_WIDTH = 60; // fits inside box (66 inner - 4 for "  - " prefix)

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
    return BatchProcessor::runBatchMode(inputFile, outputFile);
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
        "Check file errors",
        "Export error report",
        "Run assignment",
        "Run all datasets",
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

                    InputValidator validator;
                    if (!validator.validate(submissions, reviewers, params, control)) {
                        int n = (int)validator.getErrors().size();
                        lines.push_back("");
                        lines.push_back("!!" + std::to_string(n) + " validation error"
                                       + (n > 1 ? "s" : "") + " found.");
                        lines.push_back("Use 'Check file errors' for details.");
                    }
                } else {
                    lines.push_back("!!Error: Failed to parse file.");
                    lines.push_back("!!Path: \"" + filename + "\"");
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
            case 4: {
                std::vector<std::string> errLines;
                if (!loaded) {
                    errLines.push_back("!!No file loaded.");
                    errLines.push_back("Load a file first (option 1).");
                } else {
                    InputValidator validator;
                    if (validator.validate(submissions, reviewers, params, control)) {
                        errLines.push_back("No errors found.");
                        errLines.push_back("");
                        errLines.push_back("File: " + loadedFile);
                        errLines.push_back(std::to_string(submissions.size()) + " submissions, "
                                         + std::to_string(reviewers.size()) + " reviewers");
                    } else {
                        int n = (int)validator.getErrors().size();
                        errLines.push_back("!!" + std::to_string(n) + " error"
                                          + (n > 1 ? "s" : "") + " in: " + loadedFile);
                        errLines.push_back("");
                        for (const auto &err : validator.getErrors()) {
                            auto wrapped = DisplayFormatter::wrapLine("- ", err, MAX_LINE_WIDTH);
                            for (const auto &wl : wrapped) {
                                errLines.push_back(wl);
                            }
                        }
                    }
                }
                menu.displayInBox("File Validation", errLines);
                break;
            }
            case 5: {
                std::vector<std::string> exportLines;
                if (!loaded) {
                    exportLines.push_back("!!No file loaded.");
                    exportLines.push_back("Load a file first (option 1).");
                } else {
                    InputValidator validator;
                    validator.validate(submissions, reviewers, params, control);
                    const auto &errs = validator.getErrors();

                    std::string basename = fs::path(loadedFile).stem().string();
                    std::string reportPath = "assignments/" + basename + "_errors.txt";

                    std::vector<std::pair<std::string, std::vector<std::string>>> single;
                    single.push_back({fs::path(loadedFile).filename().string(),
                                      {errs.begin(), errs.end()}});
                    ErrorReporting::writeErrorReport(reportPath, single);

                    exportLines.push_back("Error report exported.");
                    exportLines.push_back("");
                    exportLines.push_back("File: " + reportPath);
                    exportLines.push_back(std::to_string(errs.size()) + " error"
                                        + (errs.size() != 1 ? "s" : "") + " found.");
                }
                menu.displayInBox("Export Error Report", exportLines);
                break;
            }
            case 6:
                menu.displayInBox("Assignment Result", runAssignment(submissions, reviewers, params, control));
                break;
            case 7:
                menu.displayInBox("All Datasets", BatchProcessor::runAllDatasets());
                break;
            case 8:
                std::cout << "\033[?25h" << Menu::CLEAR_SCREEN;
                return 0;
        }
    }

    return 0;
}
