# DA Conference Tool

## Presentation Notes

### Dinic's vs Edmonds-Karp — Assignment Differences

We upgraded from Edmonds-Karp O(V * E^2) to Dinic's O(E * sqrt(V)) for the max-flow solver.

On 3 out of 14 datasets (10, 11, 14) the specific reviewer-submission pairings differ from the reference output, but:
- The max-flow value (total assignments) is identical
- All constraints are satisfied (MinReviews, MaxReviews, domain matching)
- No missing reviews

Why it happens: when multiple reviewers have the same domain expertise, Dinic's blocking-flow DFS commits greedily to one reviewer before moving on, while Edmonds-Karp's BFS distributes one path at a time. Both are valid — the max-flow value is unique but the edge-level assignment is not.

Key point for demo: this is a well-known property of flow networks. Mention that multiple optimal solutions exist and our algorithm finds one of them.


# DA Conference Tool

## Presentation Notes

### Menu UI Design

The interactive menu redraws the full screen on each transition (e.g., after viewing results and pressing Enter to go back). This is intentional — it allows the user to always return to a clean, centered main menu, providing a clear way to backtrack through the tool's options.