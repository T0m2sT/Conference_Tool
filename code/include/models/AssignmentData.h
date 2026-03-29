#ifndef ASSIGNMENTDATA_H
#define ASSIGNMENTDATA_H

#include <vector>

/**
 * @brief Represents a single review assignment: submission -> reviewer with a matched domain.
 */
struct Assignment {
    int submissionId;
    int reviewerId;
    int matchedDomain;  // which domain caused the match
};

/**
 * @brief Represents a submission that couldn't get enough reviews.
 */
struct MissingReview {
    int submissionId;
    int domain;
    int missingCount;
};

/**
 * @brief Result of running the assignment algorithm.
 */
struct AssignmentResult {
    std::vector<Assignment> assignments;
    std::vector<MissingReview> missingReviews;
    int totalAssignments;
    bool fullySatisfied;  // true if all submissions got MinReviews
};

#endif