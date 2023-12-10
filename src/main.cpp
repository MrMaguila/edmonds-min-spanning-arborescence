#include <iostream>
#include <vector>
#include <stack>
#include <list>
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <algorithm>


#define DELETED_EDGE -1

class Graph
{
public:

	void load(const char* file)
	{
		std::ifstream ifs(file);
		int num_vertices, origin, dest, weight;
		num_vertices = origin = dest = weight = 0;

		if (!ifs.is_open() || ifs.eof()) {
			std::cerr << "Graph file not found or empty\n";
			std::terminate();
		}

		//read the number of vertices
		ifs >> num_vertices;
		assert(num_vertices > 0 && num_vertices < 100000000);

		
		adjacency_list.resize(num_vertices);

		//read each origin, destination and weight tuple
		//and save it as a single directed edge
		int num_edges = 0;
		while(ifs >> origin >> dest >> weight)
		{
			assert(origin >= 0 && origin < num_vertices);
			assert(dest >= 0 && dest < num_vertices);

			adjacency_list[origin].push_back({ dest, weight });
			++num_edges;
		}

		ifs.close();

		std::cout << "Number of vertices: " << num_vertices << "\n";
		std::cout << "Number of edges: " << num_edges << "\n";
	}

	//================================================


	/// <summary>
	/// computes and returns a new graph representing a minimum spanning tree
	/// of this graph. The method assumes all vertices are achievable from the root
	/// </summary>
	/// <param name="root"></param>
	/// <returns></returns>
	Graph min_spanning_arborescence(int root) const
	{
		Graph result = *this;

		//Cleanup: remove unneccessary edges
		for (int v = 0; v < result.adjacency_list.size(); ++v)
		{
			std::vector<EdgeInfo>& edges = result.adjacency_list[v];

			//remove loops and all edges whose destination is the root vertex
			for (EdgeInfo& edge : edges)
				if (edge.dest == root || edge.dest == v)
					edge.dest = DELETED_EDGE;

			// remove parallel edges (and keep the one with lowest weight)
			if (edges.size() > 1)
			{
				std::sort(edges.begin(), edges.end());
				for (int i = 0; i < edges.size() - 1; ++i)
				{
					if (edges[i].dest != DELETED_EDGE && edges[i].dest == edges[i + 1].dest)
					{
						int erase_index = edges[i].weight < edges[i + 1].weight
							? i + 1
							: i;

						edges[erase_index].dest = DELETED_EDGE;
					}
				}
			}
		}

		//---------------------
		// Recursive step
		result = result._min_spanning_arborescence(root);
		
		return result;
	}

	//================================================

	void save(const char* filename) const
	{
		std::ofstream ofs(filename);
		assert(ofs.is_open());

		ofs << adjacency_list.size() << std::endl;

		for (int v = 0; v < adjacency_list.size(); ++v)
		{
			for (const auto& edge : adjacency_list[v])
				if (edge.dest != DELETED_EDGE)
					ofs << v << " " << edge.dest << " " << edge.weight << std::endl;
		}
	}


	//================================================

private:


	Graph _min_spanning_arborescence(int root) const
	{
		Graph result = *this;
		if (result.adjacency_list.size() <= 1)
			return result;

		std::vector<EdgeInfo*> lowest_weight_edge(result.adjacency_list.size(), nullptr);

		// for each vertex, remove all edges incident on it, except the one with smallest weight
		for (auto& edges : result.adjacency_list)
		{
			for (EdgeInfo& edge : edges)
			{
				if (edge.dest != DELETED_EDGE && edge.dest != root)
				{
					if (lowest_weight_edge[edge.dest] == nullptr)
						lowest_weight_edge[edge.dest] = &edge;
					else if (edge.weight < lowest_weight_edge[edge.dest]->weight)
					{
						// make this the lowest weight incident edge on edge.dest, and remove the previous one
						lowest_weight_edge[edge.dest]->dest = DELETED_EDGE;
						lowest_weight_edge[edge.dest] = &edge;
					}
					else //this edge is heavier than the previous, just remove it
						edge.dest = DELETED_EDGE;
				}
			}
		}

		//--------------
		// Search for cycles

		std::map<int, int> cycle = result._get_cycle();

		// if there's no cycle, result is the correct minimum spanning arborescence
		if (cycle.size() == 0)
			return result;

		std::cout << "cycle found\n";
		for (auto& edge : cycle)
			std::cout << edge.first << ", " << edge.second << "\n";

		//-------------
		// contracting the cycle

		Graph d;
		d.adjacency_list.resize(result.adjacency_list.size() - cycle.size() + 1);

		std::map<int, int> vertex_mapping; // maps result vertices to d vertices
		std::map<std::pair<int, int>, FullEdgeInfo> edge_mapping; // maps d edges to result edges

		const int vc = d.adjacency_list.size() - 1; // a single vertex to replace the entire cycle
		int v, u, mapped_v, mapped_u;
		v = u = mapped_v = mapped_u = 0;

		// map each vertex in result to its corresponding in d
		for (int origin = 0; origin < result.adjacency_list.size(); ++origin)
			if (cycle.find(origin) == cycle.end())
				vertex_mapping[origin] = mapped_v++;
			else
				vertex_mapping[origin] = vc; // all cycle vertices are mapped to vc

		// set d edges
		for (; v < result.adjacency_list.size(); ++v)
		{
			mapped_v = vertex_mapping[v];
			bool is_v_in_cycle = mapped_v == vc;

			for (const EdgeInfo& edge : result.adjacency_list[v])
			{
				u = edge.dest;
				mapped_u = vertex_mapping[u];
				bool is_u_in_cycle = mapped_u == vc;

				if (!is_v_in_cycle && is_u_in_cycle)
				{
					//add a new edge (v, vc) in d, with weight = weight(v, u) - weight(lowest weight edge incident on u)
					edge_mapping[{mapped_v, mapped_u}] = { v, u, edge.weight };
					d.adjacency_list[mapped_v].push_back({ vc, edge.weight - lowest_weight_edge[u]->weight });
				}
				else if (is_v_in_cycle && !is_u_in_cycle)
				{
					//add a new edge (vc, u) in d, with weight = weight(v, u)
					edge_mapping[{vc, mapped_u}] = { v, u, edge.weight };
					d.adjacency_list[vc].push_back({ mapped_u, edge.weight });
				}
				else if (!is_v_in_cycle)
				{
					//add a new edge (v, u) in d, with weight = weight(v, u)
					edge_mapping[{mapped_v, mapped_u}] = { v, u, edge.weight };
					d.adjacency_list[mapped_v].push_back({ mapped_u, edge.weight });
				}
			}
		}

		//find a minimum spanning arborescence from d
		Graph child_msa = result._min_spanning_arborescence(vertex_mapping[root]);

		//---------------------------
		// remove the vc_edge from the cycle

		std::pair<int, int> vc_edge = { -1, -1 };
		for (int origin = 0; origin < child_msa.adjacency_list.size() && vc_edge.first != -1; ++origin)
		{
			for (const auto& edge : child_msa.adjacency_list[origin])
			{
				if (edge.dest == vc)
				{
					vc_edge = { origin, edge.dest };
					break;
				}
			}
		}

		cycle.erase(vertex_mapping[vc_edge.first]);

		//--------------------------------
		// contruct the final msa

		Graph msa;
		msa.adjacency_list.resize(result.adjacency_list.size());

		for (int origin = 0; origin < child_msa.adjacency_list.size(); ++origin)
		{
			for (auto& edge : child_msa.adjacency_list[origin])
			{
				auto mapped_edge = edge_mapping[{origin, edge.dest}];
				msa.adjacency_list[mapped_edge.origin].push_back({ mapped_edge.dest, mapped_edge.weight });
			}
		}

		//insert edges from the cycle
		for (const auto& cycle_edge : cycle) 
		{
			for (const auto& edge : result.adjacency_list[cycle_edge.first])
				if (edge.dest == cycle_edge.second)
					msa.adjacency_list[cycle_edge.first].push_back(edge);
		}

		return msa;
	}

	//================================================

	struct VertexSearchInfo {
		bool visited = false;
		bool in_stack = false;
		int current_edge = 0;
	};

	/// <summary>
	/// Verifies if there's is a cycle in the graph. Presumes there's 
	/// is no parallel edges
	/// </summary>
	/// <returns> 
	/// returns the set of edges of the first cycle found, if there's one, 
	/// otherwise returns an empty map
	/// </returns>
	std::map<int, int> _get_cycle() const
	{
		std::map<int, int> cycle_edges; //the edges that composes a cycle
		if (adjacency_list.size() < 2)
			return cycle_edges;

		int cycle_start = -1;
		std::stack<int> vertex_stack;
		std::vector<VertexSearchInfo> vertex_info;
		vertex_info.resize(adjacency_list.size());

		int v = 0;
		while (v < adjacency_list.size() && cycle_start == -1)
		{
			if (vertex_info[v].visited)
			{
				v++;
				continue;
			}

			vertex_stack.push(v);
			
			vertex_info[v].in_stack = true;
			while (!vertex_stack.empty() && cycle_start == -1)
			{
				int current_vertex = vertex_stack.top();

				// retrive the next edge of this vertex to analize, and increment the index
				int edge_index = vertex_info[current_vertex].current_edge++;
				
				//assert(adjacency_list.find(current_vertex) != adjacency_list.end());
				if (edge_index < adjacency_list.at(current_vertex).size())
				{
					// check the next edge of the current vertex
					int dest = adjacency_list.at(current_vertex)[edge_index].dest;
					
					if (dest != DELETED_EDGE)
					{
						VertexSearchInfo& dest_info = vertex_info[dest];

						if (!dest_info.visited)
						{
							vertex_stack.push(dest);
							dest_info.visited = true;
							dest_info.in_stack = true;
						}
						else if (dest_info.in_stack) // if (current_vertex, dest) is a back edge
						{
							cycle_start = dest;
							break;
						}
					}
				}
				else
				{
					vertex_stack.pop(); 
					vertex_info[current_vertex].in_stack = false;
				}
			}

			v++;
		}

		
		// retrieve the cycle edges from the vertex stack
		if (cycle_start != -1)
		{
			int current = vertex_stack.top();
			vertex_stack.pop();

			cycle_edges[cycle_start] = current;

			while (!vertex_stack.empty() && current != cycle_start) {
				int previous = current;
				current = vertex_stack.top();
				vertex_stack.pop();

				cycle_edges[current] = previous;
				//cycle_edges[current].dest = previous;
			}

			assert(current == cycle_start);
		}
				 
		return cycle_edges;
	}

	//================================================

	struct FullEdgeInfo
	{
		int origin;
		int dest;
		int weight;
	};

	struct EdgeInfo 
	{
		bool operator<(const EdgeInfo& other) 
		{
			return this->dest < other.dest && this->weight < other.weight;
		}

		int dest;
		int weight;
	};

	std::vector<std::vector<EdgeInfo>> adjacency_list;
};


//##########################################################

int main()
{
	Graph graph;
	std::cout << "Loading graph..." << std::endl;
	graph.load("../graphs/test_graph.txt");

	std::cout << "-----------------------------------------------------\n";

	Graph min_arborescence = graph.min_spanning_arborescence(0);
	min_arborescence.save("../result.txt");

	std::cout << "Minimum spanning arborescece saved as ./result.txt\n";

	return 0;
}