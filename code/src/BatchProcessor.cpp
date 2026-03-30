#include <iostream>
#include <algorithm>
#include <filesystem>
#include "BatchProcessor.h"
#include "parser/CSVParser.h"
#include "parser/InputValidator.h"
#include "FlowNetwork.h"
#include "OutputWriter.h"
#include "ErrorReporting.h"

namespace fs = std::filesystem;

int BatchProcessor::runBatchMode(const std::string &inputFile, const std::string &outputFile) {
    CSVParser parser;
    std::vector<Submission> submissions;
    std::vector<Reviewer> reviewers;
    Parameters params;
    Control control;

    if (!parser.parseFile(inputFile, submissions, reviewers, params, control)) {
        std::cerr << "Error: Failed to parse input file: " << inputFile << "\n";
        return 1;
    }

    InputValidator validator;
    if (!validator.validate(submissions, reviewers, params, control)) {
        validator.printErrors();
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

std::vector<std::string> BatchProcessor::runAllDatasets() {
    const std::string inputDir = "dataset/input/";
    const std::string outputDir = "assignments/";

    std::vector<std::string> csvFiles;
    for (const auto &entry : fs::directory_iterator(inputDir)) {
        if (entry.path().extension() == ".csv") {
            csvFiles.push_back(entry.path().filename().string());
        }
    }
    std::sort(csvFiles.begin(), csvFiles.end());

    if (csvFiles.empty()) {
        return {"!!No CSV files found in " + inputDir};
    }

    std::vector<std::string> summary;
    summary.push_back("Running " + std::to_string(csvFiles.size()) + " datasets...");
    summary.push_back("");

    std::vector<std::pair<std::string, std::vector<std::string>>> allFileErrors;

    for (const auto &file : csvFiles) {
        CSVParser parser;
        std::vector<Submission> subs;
        std::vector<Reviewer> revs;
        Parameters params;
        Control control;

        bool parsed = parser.parseFile(inputDir + file, subs, revs, params, control);

        std::vector<std::string> errors;
        AssignmentResult result{};
        if (parsed) {
            InputValidator validator;
            validator.validate(subs, revs, params, control);
            errors = {validator.getErrors().begin(), validator.getErrors().end()};

            FlowNetwork flowNet;
            int mode = control.generateAssignments;
            if (mode == 0) mode = 1;
            result = flowNet.buildAndSolve(subs, revs, params, mode);

            std::string outFile = outputDir + "output_" + file;
            control.outputFileName = outFile;
            std::vector<int> atRisk;
            if (control.riskAnalysis >= 1) {
                atRisk = flowNet.riskAnalysisK1(subs, revs, params, mode);
            }
            OutputWriter::write(outFile, result, control.riskAnalysis, atRisk);
        } else {
            errors.push_back("PARSE ERROR: Failed to read file");
        }

        allFileErrors.push_back({file, errors});
        summary.push_back(ErrorReporting::formatBatchLine(file, result, errors, !parsed));
    }

    std::string reportPath = outputDir + "error_report.txt";
    ErrorReporting::writeErrorReport(reportPath, allFileErrors);

    summary.push_back("");
    summary.push_back("Output files written to " + outputDir);
    summary.push_back("Error report: " + reportPath);
    return summary;
}
