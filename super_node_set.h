
#ifndef MAGS_REWRITE_SUPER_NODE_SET_H
#define MAGS_REWRITE_SUPER_NODE_SET_H

#include <vector>

#include "parallel_hashmap/phmap.h"
#include "parallel_hashmap/btree.h"
#include "mags_types.h"

class SuperNodeSet {
    public:
        std::vector<mags::NodeID> super_nodes; // vector that stores the mapping between each original node (the index) and their belonging supernode (the value)
        std::vector<int> num_vertices; // stores the number of vertices in each supernode
        std::vector<phmap::flat_hash_map<int, int>> neighbor_edge_counts; // stores a map, for each vertex, containig <neigbour to the node, number of edges between node and neighbour>

        SuperNodeSet(mags::Graph);
        
        mags::NodeID get_super_node(mags::NodeID x);
        int get_num_vertices(mags::NodeID x);
        phmap::flat_hash_map<int, int> get_neighbor_edge_counts(mags::NodeID x);
        phmap::flat_hash_map<int, int> get_neighbor_edge_counts(mags::NodeID u, mags::NodeID v);
        int get_cartesian_product(mags::NodeID u, mags::NodeID v, int u_edges, int v_edges);
        double get_cost(mags::NodeID u);
        double get_merge_cost(mags::NodeID u, mags::NodeID v);
        
        void merge(mags::NodeID u, mags::NodeID v);
        double saving(mags::NodeID u, mags::NodeID v);

    private:
        int get_unique_edges(mags::NodeID u, mags::NodeID v, int raw_edges);
        double accumulate_cost(mags::NodeID u, int num_vertices_u, const phmap::flat_hash_map<int, int> &neighbor_edge_counts); 
        void update_neighbor_edge_counts(mags::NodeID u, mags::NodeID v);
};
#endif // MAGS_REWRITE_SUPER_NODE_SET_H
