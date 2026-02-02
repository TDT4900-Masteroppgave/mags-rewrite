#include "mags/types.h"
#include "mags/super_node_set.h"

using namespace mags;

SuperNodeSet::SuperNodeSet(Graph graph) 
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
    // TODO: is it necessary to do this here? No super node will ever have a nbr here
    neighbor_edge_counts.resize(graph.size());
    for (int u = 0; u < graph.size(); u++) {
        for (int nbr : graph.at(u)) {
            neighbor_edge_counts[u][nbr] = 1;
        }
    }
}

NodeID SuperNodeSet::get_super_node(NodeID x) {
    if(super_nodes[x] != x) {
        super_nodes[x] = get_super_node(super_nodes[x]);
    }

    return super_nodes[x];
}

int SuperNodeSet::get_num_vertices(NodeID x) {
    NodeID super_node = get_super_node(x);
    
    return num_vertices[super_node];
}

phmap::flat_hash_map<int, int> SuperNodeSet::get_neighbor_edge_counts(NodeID x) {
    NodeID u_super = get_super_node(x);
    
    return neighbor_edge_counts[u_super];
}

phmap::flat_hash_map<int, int> SuperNodeSet::get_neighbor_edge_counts(NodeID u, NodeID v) {
    NodeID u_super = get_super_node(u);
    NodeID v_super = get_super_node(v);
    
    // initializes the edge counts for w to the edge counts for u
    phmap::flat_hash_map<int, int> w_neighbor_edge_counts = get_neighbor_edge_counts(u_super);
    
    // add the edge counts for v to w
    for (const auto& [nbr, num_nbr_edges] : get_neighbor_edge_counts(v_super)) {
        w_neighbor_edge_counts[nbr] += num_nbr_edges;
    }
    
    // Handles edges that used to exist between u and v. 
    if (w_neighbor_edge_counts.contains(v_super)) {
        // stores all edges between u and v in the entry u i.e. as internal edges
        w_neighbor_edge_counts[u_super] += w_neighbor_edge_counts[v_super]; 
        w_neighbor_edge_counts.erase(v_super);
    }

    return w_neighbor_edge_counts;
}

int SuperNodeSet::get_unique_edges(NodeID u, NodeID v, int num_raw_edges) {
    // if u == v, each edge inside the same component is seen from both endpoints
    return (u == v) ? (num_raw_edges / 2) : num_raw_edges;
}

int SuperNodeSet::get_cartesian_product(NodeID u, NodeID v, int num_vertices_u, int num_vertices_v) {
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

double SuperNodeSet::accumulate_cost(NodeID u, int num_vertices_u, const phmap::flat_hash_map<int, int> &neighbor_edge_counts) {
    double total_cost = 0.0;

    // iterates over a map containing neighbors to the node and the number of edges between the node and the neighbor 
    for (const auto& [neighbor, num_nbr_edges] : neighbor_edge_counts) {
        const int cartesian_product = get_cartesian_product(u, neighbor, num_vertices_u, get_num_vertices(neighbor));
        const int unique_nbr_edges  = get_unique_edges(u, neighbor, num_nbr_edges);

        // cart = 1, unique = 1, min(1-1+1=1, 1)=1
        total_cost += std::min(cartesian_product - unique_nbr_edges + 1, unique_nbr_edges);
    }
    
    return total_cost;
}

double SuperNodeSet::get_cost(NodeID u) {
    return accumulate_cost(u, get_num_vertices(u),  get_neighbor_edge_counts(u));
}

double SuperNodeSet::get_merge_cost(NodeID u, NodeID v) {
    /*
        Getter for the accuumulated cost for merging node u and v. 
        merged_neighbor_edge_counts is the union of the neighborhood to u and v. 
    */
    const int merged_num_vertices = get_num_vertices(u) + get_num_vertices(v);

    // accumulate cost for new merged node 
    return accumulate_cost(u, merged_num_vertices,  get_neighbor_edge_counts(u, v));
}

double SuperNodeSet::saving(NodeID u, NodeID v) {
    double c_u = get_cost(u);
    double c_v = get_cost(v);
    double c_w = get_merge_cost(u, v);

    return (c_u + c_v - c_w) / (c_u + c_v);
}

void SuperNodeSet::update_neighbor_edge_counts(NodeID u_super, NodeID v_super) {
    // update the edge count to the merged node w (stored in u)
    // TODO: this step can be optimized because the saving step has already performed the same calculation. 
    
    phmap::flat_hash_map<int, int> w_neighbor_edge_counts = get_neighbor_edge_counts(u_super, v_super); 
    neighbor_edge_counts[u_super] = w_neighbor_edge_counts;
    
    // delete v as neighbor
    for (auto [nbr, num_nbr_edges] : neighbor_edge_counts[v_super]) {
        neighbor_edge_counts[nbr].erase(v_super);
    }
    
    // delete v as an entry 
    neighbor_edge_counts[v_super].clear();
    
    // update the edge count where w (stored in u) is a nbr
    for (auto [nbr, num_nbr_edges] : w_neighbor_edge_counts) {
        if (nbr != u_super) {
            neighbor_edge_counts[nbr][u_super] = num_nbr_edges;
        }
    }
}

void SuperNodeSet::merge(NodeID u, NodeID v) {
    NodeID u_super = get_super_node(u);
    NodeID v_super = get_super_node(v);

    if (u_super != v_super) {
        // replaces u and v in neigbour_edge_counts to w
        update_neighbor_edge_counts(u, v);
        
        // update the number of vertices
        // does not erase v_super from num_vertices to keep the index correct in respect to the nodes indecies
        // moreover, when using get_super_node(v) we will now get u. Hence, the value in num_vertices[v] are never used
        num_vertices[u_super] += num_vertices[v_super];
        
        // merges u and v by setting them to refer to the same vertex
        super_nodes[v_super] = u_super;
    }
}