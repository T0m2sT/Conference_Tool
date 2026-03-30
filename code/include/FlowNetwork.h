#ifndef FLOWNETWORK_H
#define FLOWNETWORK_H

template <class T>
class MutablePriorityQueue;

#include "Graph.h"
#include "models/ConferenceData.h"
#include "models/AssignmentData.h"
#include <vector>
#include <string>
#include <unordered_map>

/**
 * @brief Builds a Max-Flow network from conference data and solves the review assignment problem.
 *
 * Node naming scheme (used as vertex info in Graph<std::string>):
 *   "source"       — the super source
 *   "sink"         — the super sink
 *   "sub_<id>"     — submission node
 *   "rev_<id>"     — reviewer node
 *
 * **Algorithm:** Dinic's algorithm — O(V² · E) general, O(E · √V) on unit-capacity bipartite graphs.
 *
 * This network is a 4-layer bipartite graph (source → submissions → reviewers → sink)
 * where all sub→rev edges have unit capacity, so the O(E · √V) bound applies.
 *
 * where V = 2 + |submissions| + |reviewers|, E = matching edges + source/sink edges.
 *
 * @note The max-flow value is unique, but the specific assignment of reviewers to submissions
 *       is not — when multiple reviewers share the same domain expertise, different algorithms
 *       (e.g., Edmonds-Karp vs Dinic's) may produce different but equally valid assignments.
 *       All outputs satisfy the same constraints (MinReviews, MaxReviews, domain matching).
 */
class FlowNetwork {
public:
    /**
     * @brief Build the flow network and compute the max-flow assignment.
     *
     * **Time Complexity:** O(E · √V)
     *   - Network construction: O(|S| · |R|) for edge creation
     *   - Dinic's max-flow on this unit-capacity bipartite network: O(E · √V)
     *   - Assignment extraction: O(|S| · |R|)
     *   - Space: O(V + E)
     *
     * where V = |S| + |R| + 2, E ≈ |S| · |R|.
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
     * **Time Complexity:** O(|R| · E · √V)
     *   - For each reviewer (|R| iterations):
     *     - Rebuild network excluding that reviewer: O(|S| · |R|)
     *     - Run Dinic's max-flow: O(E · √V)
     *   - Total: O(|R| · E · √V)
     *
     * where V = |S| + |R| + 2, E ≈ |S| · |R|.
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
     *
     * **Time Complexity:** O(1)
     */
    int getMatchDomain(const Submission &sub, const Reviewer &rev, int mode);

    /**
     * @brief BFS phase of Dinic's algorithm. Assigns a level to each reachable vertex
     *        in the residual graph, building the layered graph.
     *
     * **Time Complexity:** O(V + E)
     *
     * @param graph  The flow network graph
     * @param source Source vertex label
     * @param sink   Sink vertex label
     * @param level  Output map: vertex label → BFS level (-1 if unreachable)
     * @return true if sink is reachable (augmenting paths may exist)
     */
    bool bfsLevel(Graph<std::string> &graph,
                  const std::string &source,
                  const std::string &sink,
                  std::unordered_map<std::string, int> &level);

    /**
     * @brief DFS phase of Dinic's algorithm. Sends a blocking flow along the layered graph.
     *        Uses the "current edge" trick (iter) to skip exhausted edges in O(1).
     *
     * **Time Complexity:** O(V · E) per blocking flow phase (across all DFS calls in one phase)
     *
     * @param graph   The flow network graph
     * @param curr    Current vertex being explored
     * @param sink    Sink vertex label
     * @param pushed  Flow available to push (bottleneck so far)
     * @param level   Layered graph levels from bfsLevel
     * @param iter    Current-edge pointer per vertex (index into adjacency list)
     * @return Amount of flow sent (0 if no augmenting path found)
     */
    int dfsSend(Graph<std::string> &graph,
                Vertex<std::string> *curr,
                const std::string &sink,
                int pushed,
                std::unordered_map<std::string, int> &level,
                std::unordered_map<std::string, int> &iter);

    /**
     * @brief Run Dinic's algorithm on the graph.
     *
     * Alternates between BFS (build layered graph) and DFS (blocking flow) until
     * no augmenting path exists. The current-edge optimisation ensures each edge
     * is visited at most once per phase.
     *
     * **Time Complexity:** O(V² · E) general; O(E · √V) for unit-capacity bipartite networks.
     *
     * @param graph  The flow network graph
     * @param source Source vertex label
     * @param sink   Sink vertex label
     * @return The maximum flow value
     */
    int dinic(Graph<std::string> &graph,
              const std::string &source,
              const std::string &sink);
};

#endif
