#include <iostream>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <limits>
#include <sys/stat.h>
#include "parser/CSVParser.h"
#include "models/ConferenceData.h"
#include "FlowNetwork.h"
#include "OutputWriter.h"

// ANSI escape code constants
const std::string CLEAR_SCREEN    = "\033[2J\033[H";
const std::string HIGHLIGHT_ON    = "\033[7m";
const std::string HIGHLIGHT_OFF   = "\033[0m";
const std::string CURSOR_HIDE     = "\033[?25l";
const std::string CURSOR_SHOW     = "\033[?25h";
const std::string CURSOR_SAVE     = "\033[s";
const std::string CURSOR_RESTORE  = "\033[u";

// Move cursor to a specific row
std::string cursorToRow(int row) {
    return "\033[" + std::to_string(row) + ";1H";
}

const int BOX_INNER = 66;
const std::string BOX_TOP    = "╔════════════════════════════════════════════════════════════════════╗";
const std::string BOX_MID    = "╠════════════════════════════════════════════════════════════════════╣";
const std::string BOX_BOTTOM = "╚════════════════════════════════════════════════════════════════════╝";

// Helper: build a line padded to fit inside the box
std::string boxLine(const std::string &text) {
    std::string padded = text;
    if ((int)padded.size() > BOX_INNER) padded = padded.substr(0, BOX_INNER);
    while ((int)padded.size() < BOX_INNER) padded += " ";
    return "║ " + padded + " ║";
}

std::string boxLineArrow(const std::string &text) {
    std::string padded = text;
    while ((int)padded.size() < BOX_INNER+4) padded += " ";
    return "║ " + padded + " ║";
}

// Helper: build a menu line with optional highlight
std::string menuLine(const std::string &text, bool highlighted) {
    std::string padded = text;
    while ((int)padded.size() < BOX_INNER - 3) padded += " ";
    if (highlighted)
        return "║  " + HIGHLIGHT_ON + " " + padded + HIGHLIGHT_OFF + "  ║";
    return "║   " + padded + "  ║";
}

// Display content lines inside the standard box, wait for Enter
void displayInBox(const std::string &subtitle, const std::vector<std::string> &lines) {
    std::cout << CLEAR_SCREEN;
    std::cout << "\n";
    std::cout << BOX_TOP << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("                Conference Review Assignment Tool") << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << BOX_MID << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("  " + subtitle) << "\n";
    std::cout << boxLine("") << "\n";
    for (const auto &line : lines) {
        std::cout << boxLine("  " + line) << "\n";
    }
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("  Press Enter to go back...") << "\n";
    std::cout << BOX_BOTTOM << std::endl;

    // Switch to raw mode, drain everything, then wait for one Enter
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Drain any buffered chars (from cin or fd)
    tcflush(STDIN_FILENO, TCIFLUSH);

    // Wait for a single Enter
    while (getchar() != '\n') {}
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

// Draw the box with a prompt line, read input, return it
std::string promptInBox(const std::string &subtitle, const std::string &prompt) {
    std::cout << CLEAR_SCREEN;
    std::cout << "\n";
    std::cout << BOX_TOP << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("                Conference Review Assignment Tool") << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << BOX_MID << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("  " + subtitle) << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("  " + prompt) << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << BOX_BOTTOM << "\n";
    std::cout << std::flush;
    // Move cursor back up to the prompt line, after the prompt text
    int promptCol = 4 + (int)prompt.size() + 1;
    std::cout << "\033[3A\033[" << promptCol << "G";
    std::string input;
    std::cin >> input;
    // Flush the leftover \n from cin so it doesn't leak into the next screen
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return input;
}

int arrowMenu(const std::vector<std::string> &options) {
    int selected = 0;
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Draw the full menu once
    std::cout << CLEAR_SCREEN;
    std::cout << "\n";
    std::cout << BOX_TOP << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("                Conference Review Assignment Tool") << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << BOX_MID << "\n";
    std::cout << boxLine("") << "\n";
    int firstOptionRow = 8;
    for (int i = 0; i < (int)options.size(); i++) {
        std::cout << menuLine(options[i], i == selected) << "\n";
    }
    std::cout << boxLine("") << "\n";
    std::cout << boxLineArrow("  \u2191/\u2193 arrows, Enter to select") << "\n";
    std::cout << BOX_BOTTOM << "\n";
    std::cout << CURSOR_HIDE << std::flush;

    while (true) {
        char c = getchar();
        if (c == '\n') break;
        if (c == '\033') {
            getchar();
            char arrow = getchar();
            int oldSelected = selected;
            if (arrow == 'A' && selected > 0) selected--;
            if (arrow == 'B' && selected < (int)options.size() - 1) selected++;

            if (oldSelected != selected) {
                std::cout << CURSOR_SAVE;
                std::cout << cursorToRow(firstOptionRow + oldSelected);
                std::cout << menuLine(options[oldSelected], false);
                std::cout << cursorToRow(firstOptionRow + selected);
                std::cout << menuLine(options[selected], true);
                std::cout << CURSOR_RESTORE << std::flush;
            }
        }
    }

    std::cout << CURSOR_SHOW;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return selected;
}

std::vector<std::string> getSubmissionLines(const std::vector<Submission> &submissions) {
    std::vector<std::string> lines;
    if (submissions.empty()) { lines.push_back("No submissions loaded."); return lines; }
    for (const auto &s : submissions) {
        std::string line = "ID:" + std::to_string(s.id)
            + " | " + s.title
            + " | D:" + std::to_string(s.primaryDomain);
        if (s.secondaryDomain != -1) line += "/" + std::to_string(s.secondaryDomain);
        lines.push_back(line);
    }
    return lines;
}

std::vector<std::string> getReviewerLines(const std::vector<Reviewer> &reviewers) {
    std::vector<std::string> lines;
    if (reviewers.empty()) { lines.push_back("No reviewers loaded."); return lines; }
    for (const auto &r : reviewers) {
        std::string line = "ID:" + std::to_string(r.id)
            + " | " + r.name
            + " | E:" + std::to_string(r.primaryExpertise);
        if (r.secondaryExpertise != -1) line += "/" + std::to_string(r.secondaryExpertise);
        lines.push_back(line);
    }
    return lines;
}

std::vector<std::string> getSettingsLines(const Parameters &params, const Control &control) {
    std::vector<std::string> lines;
    lines.push_back("--- Parameters ---");
    lines.push_back("MinReviews/Sub: " + std::to_string(params.minReviewsPerSubmission));
    lines.push_back("MaxReviews/Rev: " + std::to_string(params.maxReviewsPerReviewer));
    lines.push_back("PrimaryRevExpertise: " + std::to_string(params.primaryReviewerExpertise));
    lines.push_back("SecondRevExpertise: " + std::to_string(params.secondaryReviewerExpertise));
    lines.push_back("PrimarySubDomain: " + std::to_string(params.primarySubmissionDomain));
    lines.push_back("SecondSubDomain: " + std::to_string(params.secondarySubmissionDomain));
    lines.push_back("");
    lines.push_back("--- Control ---");
    lines.push_back("GenerateAssignments: " + std::to_string(control.generateAssignments));
    lines.push_back("RiskAnalysis: " + std::to_string(control.riskAnalysis));
    lines.push_back("Output: " + control.outputFileName);
    return lines;
}

std::vector<std::string> runAssignment(const std::vector<Submission> &submissions,
                                       const std::vector<Reviewer> &reviewers,
                                       const Parameters &params,
                                       const Control &control) {
    std::vector<std::string> lines;
    if (submissions.empty() || reviewers.empty()) {
        lines.push_back("Please load an input file first.");
        return lines;
    }

    FlowNetwork flowNet;
    int mode = control.generateAssignments;
    if (mode == 0) mode = 1;

    AssignmentResult result = flowNet.buildAndSolve(submissions, reviewers, params, mode);

    std::vector<int> atRisk;
    if (control.riskAnalysis == 1) {
        atRisk = flowNet.riskAnalysisK1(submissions, reviewers, params, mode);
    }

    lines.push_back("Total: " + std::to_string(result.totalAssignments));
    lines.push_back("");
    if (result.fullySatisfied) {
        lines.push_back("All submissions fully assigned.");
    } else {
        lines.push_back("Missing reviews:");
        for (const auto &m : result.missingReviews) {
            lines.push_back("  Sub " + std::to_string(m.submissionId)
                + " (D:" + std::to_string(m.domain)
                + ") missing " + std::to_string(m.missingCount));
        }
    }

    if (control.riskAnalysis == 1) {
        lines.push_back("");
        if (atRisk.empty()) {
            lines.push_back("Risk (K=1): No at-risk reviewers.");
        } else {
            std::string riskLine = "Risk (K=1): ";
            for (size_t i = 0; i < atRisk.size(); i++) {
                if (i > 0) riskLine += ", ";
                riskLine += std::to_string(atRisk[i]);
            }
            lines.push_back(riskLine);
        }
    }

    if (control.generateAssignments != 0) {
        OutputWriter::write(control.outputFileName, result, control.riskAnalysis, atRisk);
        lines.push_back("");
        lines.push_back("Output: " + control.outputFileName);
    }

    return lines;
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
    std::vector<std::string> menuOptions = {
        "Load input file",
        "Show submissions",
        "Show reviewers",
        "Show parameters & control settings",
        "Run assignment",
        "Exit"
    };

    while (true) {
        int choice = arrowMenu(menuOptions);

        switch (choice) {
            case 0: {
                std::string defaultpath = "dataset/input/";
                std::string defaultextension = ".csv";
                std::string filename = promptInBox("Load Input File", "File name: ");
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
                displayInBox("Load Input File", lines);
                break;
            }
            case 1:
                displayInBox("Submissions", getSubmissionLines(submissions));
                break;
            case 2:
                displayInBox("Reviewers", getReviewerLines(reviewers));
                break;
            case 3:
                displayInBox("Parameters & Control", getSettingsLines(params, control));
                break;
            case 4:
                displayInBox("Assignment Result", runAssignment(submissions, reviewers, params, control));
                break;
            case 5:
                std::cout << CLEAR_SCREEN;
                return 0;
        }
    }

    return 0;
}
