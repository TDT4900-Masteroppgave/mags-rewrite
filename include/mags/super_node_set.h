
#ifndef MAGS_REWRITE_SUPER_NODE_SET_H
#define MAGS_REWRITE_SUPER_NODE_SET_H

#include <vector>


#include "types.h"

#include <parallel_hashmap/phmap.h>
#include <parallel_hashmap/btree.h>

using namespace mags;

class SuperNodeSet {
    SuperNodes super_nodes; // vector that stores the mapping between each original node (the index) and their belonging supernode (the value)
    std::vector<int> num_vertices; // stores the number of vertices in each supernode
    EdgeCounts neighbor_edge_counts; // stores a map, for each vertex, containig <neigbour to the node, number of edges between node and neighbour>

    public:
        SuperNodeSet(Graph);
        
        NodeID get_super_node(NodeID x);
        int get_num_vertices(NodeID x);
        phmap::flat_hash_map<int, int> get_neighbor_edge_counts(NodeID x);
        phmap::flat_hash_map<int, int> get_neighbor_edge_counts(NodeID u, NodeID v);
        int get_cartesian_product(NodeID u, NodeID v, int u_edges, int v_edges);
        double get_cost(NodeID u);
        double get_merge_cost(NodeID u, NodeID v);
        
        void merge(NodeID u, NodeID v);
        double saving(NodeID u, NodeID v);

    private:
        int get_unique_edges(NodeID u, NodeID v, int num_raw_edges);
        double accumulate_cost(NodeID u, int num_vertices_u, const phmap::flat_hash_map<int, int> &neighbor_edge_counts); 
        void update_neighbor_edge_counts(NodeID u, NodeID v);
};
#endif // MAGS_REWRITE_SUPER_NODE_SET_H
