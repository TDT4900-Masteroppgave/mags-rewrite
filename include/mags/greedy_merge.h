#ifndef MAGS_REWRITE_GREEDY_MERGE_H
#define MAGS_REWRITE_GREEDY_MERGE_H

#include <utility>
#include <vector>

#include "mags/SuperNodeSet.h"

namespace mags::gm{
    
    SuperNodeSet greedy_merge(
        Graph const& graph,
        int num_iterations,
        CandidateSet& candidate_set,
        double start_threshold = 0.5,
        double end_threshold = 0.005,
        double ratio_base = 0.01,
        double threshold_new_saving_score = -0.03);

    namespace detail {
        NodePair minPair(NodeID u, NodeID v);

        PriorityQueue get_priority_queue(CandidateSet& candidate_set);

        double merge_threshold(
            int current_iteration,
            int num_iterations,
            double start_threshold = 0.5,
            double end_threshold = 0.005,
            double ratio_base = 0.01);

        void replace(
            NodeID v,
            SuperNodeSet& super_nodes_set,
            CandidateSet& candidate_set,
            PriorityQueue& priority_queue);

        void evaluate(
            NodeID u,
            NodeID v,
            SuperNodeSet& super_nodes_set,
            CandidateSet& candidate_set,
            std::vector<NodeID>& to_remove_suv,
            std::vector<std::pair<int, double>>& to_update_suv,
            double threshold_new_saving_score = -0.03);

        void remove_candidate_v(
            NodeID u,
            NodeID v,
            CandidateSet& candidate_set,
            PriorityQueue& priority_queue);

        void update_candidate_v(
            NodeID u,
            NodeID v,
            double saving_score,
            CandidateSet& candidate_set,
            PriorityQueue& priority_queue);
    }

} // namespace mags

#endif // MAGS_REWRITE_GREEDY_MERGE_H