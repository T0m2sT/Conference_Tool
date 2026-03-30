#include <algorithm>
#include <fstream>
#include "ErrorReporting.h"

void ErrorReporting::writeErrorReport(const std::string &outputPath,
                                      std::vector<std::pair<std::string, std::vector<std::string>>> &fileErrors) {
    std::sort(fileErrors.begin(), fileErrors.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });

    std::ofstream out(outputPath);
    out << "============\n";
    out << "Error Report\n";
    out << "============\n\n";

    int totalErrors = 0;
    int filesWithErrors = 0;
    for (const auto &[file, errors] : fileErrors) {
        if (!errors.empty()) filesWithErrors++;
        totalErrors += (int)errors.size();
    }

    out << "Files analyzed: " << fileErrors.size() << "\n";
    out << "Files with errors: " << filesWithErrors << "\n";
    out << "Total errors: " << totalErrors << "\n\n";

    for (const auto &[file, errors] : fileErrors) {
        out << "--- " << file << " ---\n";
        if (errors.empty()) {
            out << "No errors.\n";
        } else {
            for (const auto &err : errors) {
                out << "- " << err << "\n";
            }
        }
        out << "\n";
    }
}

std::string ErrorReporting::formatBatchLine(const std::string &filename,
                                            const AssignmentResult &result,
                                            const std::vector<std::string> &errors,
                                            bool parseFailed) {
    if (parseFailed) {
        return "!!" + filename + " - PARSE ERROR";
    }

    std::string status = std::to_string(result.totalAssignments) + " assigned";

    if (!result.fullySatisfied) {
        status += ", " + std::to_string(result.missingReviews.size()) + " missing";
    }

    if (!errors.empty()) {
        status += ", " + std::to_string(errors.size()) + " error"
                + (errors.size() > 1 ? "s" : "");
    }

    if (result.fullySatisfied && errors.empty()) {
        status += ", OK";
    }

    std::string prefix = !errors.empty() ? "!!" : "";
    return prefix + filename + " - " + status;
}
