#include "FlowNetwork.h"
#include <queue>
#include <algorithm>
#include <climits>

int FlowNetwork::getMatchDomain(const Submission &sub, const Reviewer &rev, int mode) {
    // Mode 1: primary domain vs primary expertise only
    if (mode == 1) {
        if (sub.primaryDomain == rev.primaryExpertise)
            return sub.primaryDomain;
        return -1;
    }

    // Mode 2: primary + secondary domain vs primary expertise only
    if (mode == 2) {
        if (sub.primaryDomain == rev.primaryExpertise)
            return sub.primaryDomain;
        if (sub.secondaryDomain != -1 && sub.secondaryDomain == rev.primaryExpertise)
            return sub.secondaryDomain;
        return -1;
    }

    // Mode 3: all domain combinations vs all expertise combinations
    if (mode == 3) {
        if (sub.primaryDomain == rev.primaryExpertise)
            return sub.primaryDomain;
        if (sub.secondaryDomain != -1 && sub.secondaryDomain == rev.primaryExpertise)
            return sub.secondaryDomain;
        if (rev.secondaryExpertise != -1 && sub.primaryDomain == rev.secondaryExpertise)
            return sub.primaryDomain;
        if (sub.secondaryDomain != -1 && rev.secondaryExpertise != -1
            && sub.secondaryDomain == rev.secondaryExpertise)
            return sub.secondaryDomain;
        return -1;
    }

    return -1;
}

bool FlowNetwork::bfsLevel(Graph<std::string> &graph, const std::string &source,
                            const std::string &sink,
                            std::unordered_map<std::string, int> &level) {
    level.clear();
    for (Vertex<std::string> *v : graph.getVertexSet())
        level[v->getInfo()] = -1;

    std::queue<Vertex<std::string>*> Q;
    Vertex<std::string> *src = graph.findVertex(source);
    level[source] = 0;
    Q.push(src);

    while (!Q.empty()) {
        Vertex<std::string> *curr = Q.front(); Q.pop();
        for (Edge<std::string> *e : curr->getAdj()) {
            const std::string &destInfo = e->getDest()->getInfo();
            if (level[destInfo] == -1 && e->getWeight() - e->getFlow() > 0) {
                level[destInfo] = level[curr->getInfo()] + 1;
                Q.push(e->getDest());
            }
        }
    }
    return level[sink] != -1;
}

int FlowNetwork::dfsSend(Graph<std::string> &graph, Vertex<std::string> *curr,
                          const std::string &sink, int pushed,
                          std::unordered_map<std::string, int> &level,
                          std::unordered_map<std::string, int> &iter) {
    const std::string &info = curr->getInfo();
    if (info == sink) return pushed;

    const auto adj = curr->getAdj();

    for (int &it = iter[info]; it < (int)adj.size(); ++it) {
        Edge<std::string> *e = adj[it];
        Vertex<std::string> *dest = e->getDest();
        int residual = (int)(e->getWeight() - e->getFlow());

        if (level[dest->getInfo()] == level[info] + 1 && residual > 0) {
            int d = dfsSend(graph, dest, sink, std::min(pushed, residual), level, iter);
            if (d > 0) {
                e->setFlow(e->getFlow() + d);
                e->getReverse()->setFlow(e->getReverse()->getFlow() - d);
                return d;
            }
        }
    }
    return 0;
}

int FlowNetwork::dinic(Graph<std::string> &graph, const std::string &source, const std::string &sink) {
    int maxFlow = 0;
    std::unordered_map<std::string, int> level;

    while (bfsLevel(graph, source, sink, level)) {
        std::unordered_map<std::string, int> iter;
        for (Vertex<std::string> *v : graph.getVertexSet())
            iter[v->getInfo()] = 0;

        int f;
        while ((f = dfsSend(graph, graph.findVertex(source), sink, INT_MAX, level, iter)) > 0)
            maxFlow += f;
    }
    return maxFlow;
}

AssignmentResult FlowNetwork::buildAndSolve(
    const std::vector<Submission> &submissions,
    const std::vector<Reviewer> &reviewers,
    const Parameters &params,
    int mode)
{
    Graph<std::string> graph;
    AssignmentResult result;
    result.totalAssignments = 0;
    result.fullySatisfied = true;

    // --- Build the flow network ---
    graph.addVertex("source");
    graph.addVertex("sink");

    // Add submission nodes + source -> submission edges
    for (const auto &sub : submissions) {
        std::string subNode = "sub_" + std::to_string(sub.id);
        graph.addVertex(subNode);
        graph.addBidirectionalEdge("source", subNode, params.minReviewsPerSubmission);
    }

    // Add reviewer nodes + reviewer -> sink edges
    for (const auto &rev : reviewers) {
        std::string revNode = "rev_" + std::to_string(rev.id);
        graph.addVertex(revNode);
        graph.addBidirectionalEdge(revNode, "sink", params.maxReviewsPerReviewer);
    }

    // Add submission -> reviewer edges based on domain matching
    // Store the match domain for each edge so we can recover it later
    std::unordered_map<std::string, int> edgeMatchDomain;

    for (const auto &sub : submissions) {
        std::string subNode = "sub_" + std::to_string(sub.id);
        for (const auto &rev : reviewers) {
            int matchDomain = getMatchDomain(sub, rev, mode);
            if (matchDomain != -1) {
                std::string revNode = "rev_" + std::to_string(rev.id);
                graph.addBidirectionalEdge(subNode, revNode, 1);
                std::string edgeKey = subNode + "->" + revNode;
                edgeMatchDomain[edgeKey] = matchDomain;
            }
        }
    }

    // --- Initialize all edge flows to 0 ---
    for (auto v : graph.getVertexSet()) {
        for (auto e : v->getAdj()) {
            e->setFlow(0);
        }
    }

    // --- Run Dinic's algorithm ---
    int totalRequired = (int)submissions.size() * params.minReviewsPerSubmission;
    int maxFlow = dinic(graph, "source", "sink");

    // This is theoretically redundant with the per-submission check below,
    // but kept for clarity and correctness validation
    if (maxFlow < totalRequired) {
        result.fullySatisfied = false;
    }

    // --- Extract assignments from flow ---
    for (const auto &sub : submissions) {
        std::string subNode = "sub_" + std::to_string(sub.id);
        auto subVertex = graph.findVertex(subNode);
        int assignedCount = 0;

        for (auto edge : subVertex->getAdj()) {
            std::string destInfo = edge->getDest()->getInfo();
            // Only submission -> reviewer edges (skip reverse to source)
            if (destInfo.substr(0, 4) == "rev_" && edge->getFlow() > 0) {
                int revId = std::stoi(destInfo.substr(4));
                std::string edgeKey = subNode + "->" + destInfo;
                int matchDomain = edgeMatchDomain[edgeKey];
                result.assignments.push_back({sub.id, revId, matchDomain});
                assignedCount++;
            }
        }

        if (assignedCount < params.minReviewsPerSubmission) {
            result.fullySatisfied = false;
            result.missingReviews.push_back({
                sub.id,
                sub.primaryDomain,
                params.minReviewsPerSubmission - assignedCount
            });
        }
    }

    result.totalAssignments = (int)result.assignments.size();

    // Sort assignments by submission ID, then reviewer ID
    std::sort(result.assignments.begin(), result.assignments.end(),
              [](const Assignment &a, const Assignment &b) {
                  return a.submissionId < b.submissionId ||
                         (a.submissionId == b.submissionId && a.reviewerId < b.reviewerId);
              });

    std::sort(result.missingReviews.begin(), result.missingReviews.end(),
              [](const MissingReview &a, const MissingReview &b) {
                  return a.submissionId < b.submissionId;
              });

    return result;
}

std::vector<int> FlowNetwork::riskAnalysisK1(
    const std::vector<Submission> &submissions,
    const std::vector<Reviewer> &reviewers,
    const Parameters &params,
    int mode)
{
    std::vector<int> atRiskReviewers;

    for (const auto &removedRev : reviewers) {
        // Build a reduced reviewer list without this reviewer
        std::vector<Reviewer> reducedReviewers;
        for (const auto &rev : reviewers) {
            if (rev.id != removedRev.id) {
                reducedReviewers.push_back(rev);
            }
        }

        // Re-run the full assignment
        AssignmentResult result = buildAndSolve(submissions, reducedReviewers, params, mode);

        if (!result.fullySatisfied) {
            atRiskReviewers.push_back(removedRev.id);
        }
    }

    std::sort(atRiskReviewers.begin(), atRiskReviewers.end());
    return atRiskReviewers;
}
