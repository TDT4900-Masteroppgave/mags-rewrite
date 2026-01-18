#include "mags_types.h"
#include "super_node_set.h"

SuperNodeSet::SuperNodeSet(mags::Graph graph) 
: super_nodes(graph.size(), 0)
, num_vertices(graph.size(), 1) // each supernode starts by containing only one vertex 
{
    // TODO: it is more readable like this, but both first layer for loops are identical and can be collapsed

    // sets the mapping between each vertex and their supernode
    for (int i = 0; i < graph.size(); i++) {
        super_nodes[i] = i;
    }
    
    // initializes edge count for between each node and their neighbor to 1, 
    // no entry is created for nodes that does not have any neighbor
    neighbor_edge_counts.resize(graph.size());
    for (int u = 0; u < graph.size(); u++) {
        for (int nbr : graph.at(u)) {
            neighbor_edge_counts[u][nbr] = 1;
        }
    }
}

mags::NodeID SuperNodeSet::get_super_node(mags::NodeID x) {
    return (x == super_nodes[x]) ? x : (super_nodes[x] == get_super_node(super_nodes[x]));
}

int SuperNodeSet::get_num_vertices(mags::NodeID x) {
    mags::NodeID super_node = get_super_node(x);
    
    return num_vertices[super_node];
}

phmap::flat_hash_map<int, int> SuperNodeSet::get_neighbor_edge_counts(mags::NodeID x) {
    mags::NodeID u_super = get_super_node(x);
    
    return neighbor_edge_counts[u_super];
}

phmap::flat_hash_map<int, int> SuperNodeSet::get_neighbor_edge_counts(mags::NodeID u, mags::NodeID v) {
    mags::NodeID u_super = get_super_node(u);
    mags::NodeID v_super = get_super_node(v);
    
    phmap::flat_hash_map<int, int> w_neighbor_edge_counts = get_neighbor_edge_counts(u_super);

    for (const auto& [nbr, num_nbr_edges] : get_neighbor_edge_counts(v_super)) {
        w_neighbor_edge_counts[nbr] += num_nbr_edges;
    }

    // Handles edges that used to exist between u and v. 
    if (w_neighbor_edge_counts.contains(v_super)) {
        w_neighbor_edge_counts[u_super] += w_neighbor_edge_counts[v_super]; // stores all edges between u and v in the entry u
        w_neighbor_edge_counts.erase(v_super);
    }

    return w_neighbor_edge_counts;
}

int SuperNodeSet::get_unique_edges(mags::NodeID u, mags::NodeID v, int raw_edges) {
    // if u == v, each edge inside the same component is seen from both endpoints
    return (u == v) ? (raw_edges / 2) : raw_edges;
}

int SuperNodeSet::get_cartesian_product(mags::NodeID u, mags::NodeID v, int num_vertices_u, int num_vertices_v) {
    /*
        Getter for the cartesian product between two nodes.
        The cartesian product between two nodes corresponds to all possible combinations of edges. 
    */
    
    if (u == v) { 
        // uses the formula for number of unique pair of nodes in one single group
        return num_vertices_u * (num_vertices_u  - 1) / 2;
    } else {
        // uses the formula for number of unique pair of nodes between two groups
        return num_vertices_u * num_vertices_v;
    }
}

double SuperNodeSet::accumulate_cost(mags::NodeID u, int num_vertices_u, const phmap::flat_hash_map<int, int> &neighbor_edge_counts) {
    double total_cost = 0.0;

    // iterates over a map containing neighbors to the node and the number of edges between the node and the neighbor 
    for (const auto& [neighbor, num_nbr_edges] : neighbor_edge_counts) {
        const int cartesian_product = get_cartesian_product(u, neighbor, num_vertices_u, get_num_vertices(neighbor));
        const int unique_nbr_edges  = get_unique_edges(u, neighbor, num_nbr_edges);

        total_cost += std::min(cartesian_product - unique_nbr_edges + 1, unique_nbr_edges);
    }

    return total_cost;
}

double SuperNodeSet::get_cost(mags::NodeID u, const phmap::flat_hash_map<int, int> &neighbor_edge_counts) {
    return accumulate_cost(u, get_num_vertices(u), neighbor_edge_counts);
}

double SuperNodeSet::get_merge_cost(mags::NodeID u, mags::NodeID v, const phmap::flat_hash_map<int, int> &merged_neighbor_edge_counts) {
    /*
        Getter for the accuumulated cost for merging node u and v. 
        merged_neighbor_edge_counts is the union of the neighborhood to u and v. 
    */
    const int merged_num_vertices = get_num_vertices(u) + get_num_vertices(v);

    // accumulate cost for new merged node 
    return accumulate_cost(u, merged_num_vertices, merged_neighbor_edge_counts);
}

double SuperNodeSet::saving(mags::NodeID u, mags::NodeID v) {
    double c_u = get_cost(u, get_neighbor_edge_counts(u));
    double c_v = get_cost(v, get_neighbor_edge_counts(v));
    double c_w = get_merge_cost(u, v, get_neighbor_edge_counts(u, v));

    return (c_u + c_v - c_w) / (c_u + c_v);
}