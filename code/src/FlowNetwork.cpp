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

bool FlowNetwork::bfs(Graph<std::string> &graph, const std::string &source, const std::string &sink) {
    for (Vertex<std::string> *v : graph.getVertexSet()) {
        v->setVisited(false);
        v->setPath(nullptr);
    }

    std::queue<Vertex<std::string>*> Q;
    Vertex<std::string> *sourceVertex = graph.findVertex(source);
    sourceVertex->setVisited(true);
    Q.push(sourceVertex);

    while (!Q.empty()) {
        Vertex<std::string> *current = Q.front();
        Q.pop();

        for (Edge<std::string> *e : current->getAdj()) {
            Vertex<std::string> *dest = e->getDest();
            int residualCapacity = e->getWeight() - e->getFlow();

            if (!dest->isVisited() && residualCapacity > 0) {
                dest->setVisited(true);
                dest->setPath(e);
                Q.push(dest);

                if (dest->getInfo() == sink) return true;
            }
        }
    }

    return false;
}

int FlowNetwork::edmondsKarp(Graph<std::string> &graph, const std::string &source, const std::string &sink) {
    int maxFlow = 0;

    // Keep finding augmenting paths until none exist
    while (bfs(graph, source, sink)) {
        // Find bottleneck: walk backwards from sink to source
        double bottleneck = INF;
        auto v = graph.findVertex(sink);
        while (v->getInfo() != source) {
            Edge<std::string> *e = v->getPath();
            double residual = e->getWeight() - e->getFlow();
            if (residual < bottleneck) bottleneck = residual;
            v = e->getOrig();
        }

        // Push flow along the path
        v = graph.findVertex(sink);
        while (v->getInfo() != source) {
            Edge<std::string> *e = v->getPath();
            e->setFlow(e->getFlow() + bottleneck);
            e->getReverse()->setFlow(e->getReverse()->getFlow() - bottleneck);
            v = e->getOrig();
        }

        maxFlow += (int)bottleneck;
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

    // --- Run Edmonds-Karp ---
    int totalRequired = (int)submissions.size() * params.minReviewsPerSubmission;
    int maxFlow = edmondsKarp(graph, "source", "sink");

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
