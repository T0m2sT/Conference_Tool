#ifndef FLOWNETWORK_H
#define FLOWNETWORK_H

template <class T>
class MutablePriorityQueue;

#include "Graph.h"
#include "models/ConferenceData.h"
#include <vector>
#include <string>
#include <unordered_map>

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

/**
 * @brief Builds a Max-Flow network from conference data and solves the review assignment problem.
 *
 * Node naming scheme (used as vertex info in Graph<std::string>):
 *   "source"       — the super source
 *   "sink"         — the super sink
 *   "sub_<id>"     — submission node
 *   "rev_<id>"     — reviewer node
 *
 * Time complexity of buildAndSolve: O(V * E^2) for Edmonds-Karp,
 * where V = 2 + |submissions| + |reviewers|, E = edges based on domain matching.
 */
class FlowNetwork {
public:
    /**
     * @brief Build the flow network and compute the max-flow assignment.
     *
     * @param submissions  Parsed submissions
     * @param reviewers    Parsed reviewers
     * @param params       Problem parameters
     * @param mode         GenerateAssignments mode (1, 2, or 3)
     * @return AssignmentResult containing assignments and any missing reviews
     */
    AssignmentResult buildAndSolve(
        const std::vector<Submission> &submissions,
        const std::vector<Reviewer> &reviewers,
        const Parameters &params,
        int mode
    );

    /**
     * @brief Run risk analysis for K=1.
     * Tests each reviewer's removal and checks if all submissions still get MinReviews.
     *
     * @param submissions  Parsed submissions
     * @param reviewers    Parsed reviewers
     * @param params       Problem parameters
     * @param mode         GenerateAssignments mode
     * @return Sorted vector of reviewer IDs whose removal breaks the assignment
     */
    std::vector<int> riskAnalysisK1(
        const std::vector<Submission> &submissions,
        const std::vector<Reviewer> &reviewers,
        const Parameters &params,
        int mode
    );

private:
    /**
     * @brief Check if a reviewer can review a submission given the current mode.
     * @return The matched domain (>0) if compatible, -1 if not.
     */
    int getMatchDomain(const Submission &sub, const Reviewer &rev, int mode);

    /**
     * @brief Run Edmonds-Karp (BFS-based Ford-Fulkerson) on the graph.
     * @param graph  The flow network graph
     * @param source Source vertex label
     * @param sink   Sink vertex label
     * @return The maximum flow value
     *
     * Time complexity: O(V * E^2)
     */
    int edmondsKarp(Graph<std::string> &graph,
                    const std::string &source,
                    const std::string &sink);

    // TODO(human): BFS that finds an augmenting path in the residual graph
    bool bfs(Graph<std::string> &graph,
             const std::string &source,
             const std::string &sink);
};

#endif
