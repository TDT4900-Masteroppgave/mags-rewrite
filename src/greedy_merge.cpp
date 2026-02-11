#include "mags/greedy_merge.h"

#include <cmath>
#include <utility>
#include <vector>

#include "parallel_hashmap/btree.h"
#include "parallel_hashmap/phmap.h"

#include "mags/SuperNodeSet.h"
#include "mags/types.h"

#include <ranges>

namespace mags::gm {
namespace detail {

NodePair minPair(const NodeID u, const NodeID v) {
  return u < v ? std::make_pair(u, v) : std::make_pair(v, u);
}

PriorityQueue get_priority_queue(const CandidateSet &candidate_set) {
  PriorityQueue priority_queue;

  for (int u = 0; u < static_cast<int>(candidate_set.size());
       u++) { // loops through each node
    // loops through candidates to node and their saving
    for (const auto& [v, saving_score] : candidate_set[u]) {
      if (u > v) {
        // processes only the upper triangle to avoid inserting duplicates
        // Inserts (saving_score, u, v) into the priority queue
        priority_queue.emplace(saving_score, minPair(u, v));
      }
    }
  }

  return priority_queue;
}

double merge_threshold(const int current_iteration, const int num_iterations,
                       const double start_threshold, const double end_threshold,
                       const double ratio_base) {
  if (current_iteration == num_iterations) {
    return end_threshold;
  }

  // current_iteration < num_iterations
  const double r = std::pow(ratio_base, 1.0 / (num_iterations - 1));
  return start_threshold * std::pow(r, current_iteration - 1);
}

void replace(const NodeID v, const SuperNodeSet &super_nodes_set,
             CandidateSet &candidate_set, PriorityQueue &priority_queue) {
  // v is a node to remove, and u is its new representative
  // v is intentionally used in the code to remove old entries
  const NodeID u_super = super_nodes_set.get_super_node(v);

  for (const auto& [candidate_node, saving_score] : candidate_set[v]) {
    // removes invalid candidates (v, candidate_node) from the priority queue
    // and candidate set

    priority_queue.erase(
        priority_queue.find({saving_score, minPair(v, candidate_node)}));

    // for all candidate_nodes, remove the invalid node v as a candidate
    candidate_set[candidate_node].erase(v);

    if (candidate_node == u_super)
      continue;

    if (!candidate_set[u_super].contains(candidate_node)) {
      // inserts placeholders into the candidate set and the priority queue
      // necessary to always keep every candidate pair in the candidate set and
      // priority queue
      candidate_set[u_super][candidate_node] = -1.0;
      candidate_set[candidate_node][u_super] = -1.0;
      priority_queue.emplace(-1.0, minPair(u_super, candidate_node));
    }
  }
  // removes node v from the candidate set
  candidate_set[v].clear();
}

void evaluate(const NodeID u, const NodeID v, const SuperNodeSet &super_nodes_set,
              CandidateSet &candidate_set, std::vector<NodeID> &to_remove_suv,
              std::vector<std::pair<int, double>> &to_update_suv,
              const double threshold_new_saving_score) {
  // compute new saving
  const double new_saving_score = super_nodes_set.saving(u, v);

  // early termination
  // Floating point comparison is tricky, but here strict equality check is used as optimization
  // for "no change".
  if (std::abs(candidate_set[u][v] - new_saving_score) < 1e-9)
    return;

  // remove v if saving is less than the threshold
  if (new_saving_score <= threshold_new_saving_score) {
    to_remove_suv.push_back(v);
  } else {
    // update v if saving is equal or greater than the threshold
    // old_saving_score is pushed to identify the PQ when updating it
    to_update_suv.emplace_back(v, candidate_set[u][v]);

    // updates saving score to candidate_set[u][v] (because of reference)
    candidate_set[u][v] = new_saving_score;
  }
}

void remove_candidate_v(const NodeID u, const NodeID v, CandidateSet &candidate_set,
                        PriorityQueue &priority_queue) {
  // removes the entry (suv(u, v), u, v) from the priority queue
  priority_queue.erase(
      priority_queue.find({candidate_set[u][v], minPair(u, v)}));
  // removes v as a candidate for u
  candidate_set[u].erase(v);
  // delete symmetric entry
  candidate_set[v].erase(u);
}

void update_candidate_v(const NodeID u, const NodeID v, double saving_score,
                        CandidateSet &candidate_set,
                        PriorityQueue &priority_queue) {
  // the update loop over u has previously updated candidate_set[u][v], ensures
  // that the saving score is the same in both directions
  candidate_set[v][u] = candidate_set[u][v];
  // erase the old entry with the old out-of-date
  // saving score from the priority queue
  priority_queue.erase(priority_queue.find({saving_score, minPair(u, v)}));
  // update the priority queue with the new saving score stored in the
  // candidate set (the new saving score is stored in evaluate)
  priority_queue.insert({candidate_set[u][v], minPair(u, v)});
}
} // namespace detail

SuperNodeSet greedy_merge(Graph const &graph, const int num_iterations,
                          CandidateSet &candidate_set,
                          const double start_threshold,
                          const double end_threshold,
                          const double ratio_base,
                          const double threshold_new_saving_score) {
  // Line 1: Initialize the set of supernodes
  SuperNodeSet super_nodes_set(graph);
  // Line 2: Create a priority queue for the candidate nodes.
  // The priority queue should have the format (s(u, v), u, v)
  PriorityQueue priority_queue = detail::get_priority_queue(candidate_set);

  // Pre-allocate buffers for loop reuse
  std::vector<NodeID> batch_to_remove;
  phmap::flat_hash_set<NodeID> batch_to_update;
  std::vector<NodeID> to_remove_suv;
  std::vector<std::pair<int, double>> to_update_suv;

  // Line 3: Loops over all iterations
  for (int i = 1; i <= num_iterations; i++) {
    batch_to_remove.clear();
    batch_to_update.clear();

    const double current_threshold = detail::merge_threshold(i, num_iterations, start_threshold,
                                      end_threshold, ratio_base);

    // Line 4: Loops through the priority queue, by processing the elements with
    // highest saving first
    for (const auto &[s, node_pair] : priority_queue) {

      auto [u, v] = node_pair;

      // Line 5: break for previous saving (early termination)
      if (s < current_threshold)
        continue;

      // The previous saving can be out of date do to the previous merges, hence
      // the current saving is also checked
      // Line 6: break for current saving
      if (super_nodes_set.saving(u, v) >= current_threshold) {
        // Line 7: Merge u and v into w in the set of super nodes
        super_nodes_set.merge(u, v);

        // Add v to be removed from the candidate set and the priority queue
        batch_to_remove.push_back(v);
        // Add w (stored in u) to be updated from the
        // candidate set and the priority queue
        batch_to_update.insert(u);
      }
    }
    // Line 8: Replaces u and v by w in candidate set and priority queue
    for (const NodeID v : batch_to_remove) {
      detail::replace(v, super_nodes_set, candidate_set, priority_queue);
    }

    // Line 9: Define a set of nodes that include the newly merged node w and
    // w's neighborhood
    // Note: iterating a copy of batch_to_update to modify it? No, we need new neighbors.
    // Optimization: avoid collecting large set if possible?
    // We can't modify batch_to_update while iterating.

    // We collect ALL neighbors of ALL updated nodes.
    phmap::flat_hash_set<NodeID> neighbors;
    for (const NodeID w : batch_to_update) {
      if (super_nodes_set.get_super_node(w) != w)
        // only for safety: if w not is the current super node, then skip
        continue;

      const auto& w_neighbors = super_nodes_set.get_neighbor_edge_counts(w);
      for (const auto& [nbr, _] : w_neighbors) {
        neighbors.insert(nbr);
      }
    }
    // creates a union set of batch_to_update and neighbors
    // Note: phmap::flat_hash_set merge moves elements.
    for (const auto& nbr : neighbors) {
        batch_to_update.insert(nbr);
    }

    // Line 10: Loops over all nodes in the set of nodes defined in Line 9
    for (const NodeID &u : batch_to_update) {
      to_remove_suv.clear();
      to_update_suv.clear();

      // Line 11: Loops over candidate pairs in the candidate set containing the
      // node in Line 10
      for (const auto &v : candidate_set[u] | std::views::keys) {
        if (batch_to_update.contains(v) && u > v)
          continue; // process only the upper triangle

        // Marks the candidate pairs to be removed or updated in Line 12
        detail::evaluate(u, v, super_nodes_set, candidate_set, to_remove_suv,
                         to_update_suv, threshold_new_saving_score);
      }
      // Line 12: Update the saving belonging to the candidate pairs in Line 11
      for (const auto v : to_remove_suv) {
        detail::remove_candidate_v(u, v, candidate_set, priority_queue);
      }
      for (const auto &[v, saving_score] : to_update_suv) {
        detail::update_candidate_v(u, v, saving_score, candidate_set,
                                   priority_queue);
      }
    }
  }

  return super_nodes_set;
}
} // namespace mags::gm
