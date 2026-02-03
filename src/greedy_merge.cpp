#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>
#include <functional>
#include <queue>

#include "parallel_hashmap/phmap.h"
#include "parallel_hashmap/btree.h"
#include "parallel_hashmap/phmap_fwd_decl.h"

#include "mags/types.h"
#include "mags/super_node_set.h"

namespace mags::gm {
    namespace detail {

        NodePair minPair(NodeID u, NodeID v) {
            return u < v ? std::make_pair(u, v) : std::make_pair(v, u);
        }
    
        PriorityQueue get_priority_queue(CandidateSet& candidate_set) {
                PriorityQueue priority_queue;
    
                    
                for (int u = 0; u < static_cast<int>(candidate_set.size()); u++) { // loops through each node
                    // loops through candidates to node and their saving
                    for (auto [v, saving_score] : candidate_set[u]) { 
                        if (u > v) { // processes only upper triangle to avoid inserting duplicates 
                            // Inserts (saving_score, u, v) into the priority queue 
                            priority_queue.emplace(saving_score, minPair(u, v));
                        }
                    }
                }
    
                return priority_queue;
        }
    
        double merge_threshold(
            int current_iteration,
            int num_iterations, 
            double start_threshold = 0.5, 
            double end_threshold = 0.005, 
            double ratio_base = 0.01) {
                if (current_iteration == num_iterations) {
                    return end_threshold;
                } 
                
                // current_iteration < num_iterations
                double r = std::pow( ratio_base, 1.0/(num_iterations - 1));
                return start_threshold * std::pow(r, current_iteration - 1);
        }
    
        void replace(
            NodeID v,
            SuperNodeSet& super_nodes_set,
            CandidateSet& candidate_set,
            PriorityQueue& priority_queue) {
                // v is a node to remove, and u is its new representative
                // v is intentionally used in the code to remove old entries
                NodeID u_super = super_nodes_set.get_super_node(v);
                
                for (auto [candidate_node, saving_score] : candidate_set[v]) {
                    // removes invalid candidates (v, candidate_node) from the priority queue and candidate set
                    priority_queue.erase(priority_queue.find({saving_score, minPair(v, candidate_node)}));
                    
                    // for all candidate_nodes, remove the invalid node v as a candidate
                    candidate_set[candidate_node].erase(v);
                    
                    if (candidate_node == u_super) continue;
                    
                    if (!candidate_set[u_super].contains(candidate_node)) {
                        // inserts placeholders into the candidate set and the priority queue
                        // necessary to always keep every candidate pair in the candidate set and priority queue 
                        candidate_set[u_super][candidate_node] = candidate_set[candidate_node][u_super] = -1;
                        priority_queue.emplace( -1, minPair(u_super, candidate_node));
                    }
                }
                // removes node v from the candidate set
                candidate_set[v].clear();
        }
    
        void evaluate(
            NodeID u,
            NodeID v, 
            SuperNodeSet& super_nodes_set,
            CandidateSet& candidate_set,
            std::vector<NodeID>& to_remove_suv, 
            std::vector<std::pair<int, double>>& to_update_suv,
            double threshold_new_saving_score = -0.03) {
                // compute new saving
                double new_saving_score = super_nodes_set.saving(u, v);
    
                // early termination
                if (candidate_set[u][v] == new_saving_score) return;
    
                if (new_saving_score <= threshold_new_saving_score) { // remove v if saving is less than threshold
                    to_remove_suv.push_back(v);
                } else { // update v if saving is equal or greater than threshold
                    to_update_suv.push_back({v, candidate_set[u][v]}); // here old_saving_score is pushed to identify the PQ when updating it 
                    candidate_set[u][v] = new_saving_score; // updates saving score to candidate_set[u][v] (because of reference)
                }
        }
    
        void remove_candidate_v(
            NodeID u, 
            NodeID v,
            CandidateSet& candidate_set,
            PriorityQueue& priority_queue) {
                // removes the entry (suv(u, v), u, v) from the priority queue
                priority_queue.erase(priority_queue.find({candidate_set[u][v], minPair(u,v)}));
                // removes v as a candidate for u
                candidate_set[u].erase(v);
                // delete symmetric entry
                candidate_set[v].erase(u);
        }
    
        void update_candidate_v(
            NodeID u, 
            NodeID v,
            double saving_score,
            CandidateSet& candidate_set,
            PriorityQueue& priority_queue) {
                // the update loop over u has previous updated candidate_set[u][v], ensures that the saving score is the same in both directions
                candidate_set[v][u] = candidate_set[u][v];
                // erase the old entry with the old out-of-date saving score from the priority queue
                priority_queue.erase(priority_queue.find({saving_score, minPair(u, v)}));
                // update the priority queue with the new saving score stored in candidate set (the new saving score is stored in evaluate)
                priority_queue.insert({candidate_set[u][v], minPair(u, v)});
        }
    }

    SuperNodeSet greedy_merge(
        Graph const& graph, 
        int num_iterations,
        CandidateSet& candidate_set, 
        double start_threshold = 0.5, 
        double end_threshold = 0.005, 
        double ratio_base = 0.01,
        double threshold_new_saving_score = -0.03
        ) {
        // Line 1: Initialize set of supernodes
        SuperNodeSet super_nodes_set(graph);
        // Line 2: Create a priority queue for the candidate nodes. 
        // The priority queue should have the format (s(u, v), u, v)
        PriorityQueue priority_queue = detail::get_priority_queue(candidate_set);
        
        // Line 3: Loops over all iterations 
        for (int i = 1; i <= num_iterations; i++) {
            std::vector<NodeID> batch_to_remove;
            phmap::flat_hash_set<NodeID> batch_to_update; // set containing unique nodes and their neighbors

            // Line 4: Loops through the priority queue, by processing the elements with highest saving first
            for (const auto& [s, node_pair] : priority_queue) {
                
                auto [u, v] = node_pair;

                // Line 5: break for previous saving (early termination)
                if (s < detail::merge_threshold(i, num_iterations)) continue;

                // The previous saving can be out of date do to the previous merges, hence the current saving are also checked
                // Line 6: break for current saving
                if (super_nodes_set.saving(u, v) >= detail::merge_threshold(i, num_iterations)) {
                    // Line 7: Merge u and v into w in the set of super nodes
                    super_nodes_set.merge(u,v);
                    
                    // Add v to be removed from the cadidate set and the priority queue
                    batch_to_remove.push_back(v);
                    // Add w (stored in u) to be updated from the cadidate set and the priority queue
                    batch_to_update.insert(u);
                }
            }
            // Line 8: Replaces u and v by w in candidate set and priority queue
            for (NodeID v : batch_to_remove) {
                detail::replace(v, super_nodes_set, candidate_set, priority_queue);
            }
            
            // Line 9: Define a set of nodes that include the newly merged node w and w's neighborhood
            phmap::flat_hash_set<NodeID> neighbors;
            for (NodeID w : batch_to_update) {
                if (super_nodes_set.get_super_node(w) != w) continue; // only for saftely: if w not is the current super node, then skip
                for (auto [nbr, _] : super_nodes_set.get_neighbor_edge_counts(w)) {
                    neighbors.insert(nbr);
                }
            }
            // creates a union set of batch_to_update and neighbors
            batch_to_update.merge(neighbors);

            // Line 10: Loops over all nodes in the set of nodes defined in Line 9
            for (const NodeID& u : batch_to_update) {
                std::vector<NodeID> to_remove_suv;
                std::vector<std::pair<int, double>> to_update_suv; 

                // Line 11: Loops over canidate pairs in the candidate set containing the node in Line 10 
                for (auto& [v, old_saving_score] : candidate_set[u]) {
                    if (batch_to_update.contains(v) && u > v) continue; // process only upper triangle
                    
                    // Marks the candidate pairs to be removed or updated in Line 12
                    detail::evaluate(u, v, super_nodes_set, candidate_set, to_remove_suv, to_update_suv, threshold_new_saving_score);
                }
                // Line 12: Update the saving belonging to the candidate pairs in Line 11
                for (auto v : to_remove_suv) {
                    detail::remove_candidate_v(u, v, candidate_set, priority_queue);
                }
                for (auto& [v, saving_score] : to_update_suv) {
                    detail::update_candidate_v(u, v, saving_score, candidate_set, priority_queue);
                }

            }
        }

        return super_nodes_set;
    }
}