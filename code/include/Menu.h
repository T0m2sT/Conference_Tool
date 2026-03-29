#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>
#include "models/ConferenceData.h"

class Menu {
public:
    void displayInBox(const std::string &subtitle, const std::vector<std::string> &lines);
    std::string promptInBox(const std::string &subtitle, const std::string &prompt);
    int arrowMenu(const std::vector<std::string> &options);

    static std::vector<std::string> getSubmissionLines(const std::vector<Submission> &submissions);
    static std::vector<std::string> getReviewerLines(const std::vector<Reviewer> &reviewers);
    static std::vector<std::string> getSettingsLines(const Parameters &params, const Control &control);
    static std::vector<std::string> runAssignment(const std::vector<Submission> &submissions,
                                                  const std::vector<Reviewer> &reviewers,
                                                  const Parameters &params,
                                                  const Control &control);

    static const std::string CLEAR_SCREEN;

private:
    static const std::string HIGHLIGHT_ON;
    static const std::string HIGHLIGHT_OFF;
    static const std::string CURSOR_HIDE;
    static const std::string CURSOR_SHOW;
    static const std::string CURSOR_SAVE;
    static const std::string CURSOR_RESTORE;

    static const int BOX_INNER;
    static const std::string BOX_TOP;
    static const std::string BOX_MID;
    static const std::string BOX_BOTTOM;

    static std::string cursorToRow(int row);
    static std::string boxLine(const std::string &text);
    static std::string boxLineArrow(const std::string &text);
    static std::string menuLine(const std::string &text, bool highlighted);
};

#endif
