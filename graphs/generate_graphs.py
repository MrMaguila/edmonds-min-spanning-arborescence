import networkx as nx
import random

random.seed(123)

graph = nx.DiGraph()
graph.add_nodes_from(range(15))
graph.add_edges_from([
    (
      random.randrange(0, 15), 
      random.randrange(0, 15),
      {"weight": random.randrange(0, 100)}
    ) for _ in range(100)
])


print(graph.number_of_nodes())
nx.write_edgelist(graph, "test_graph.txt", data=["weight"])


#save the number of vertices in the file first line
with open('test_graph.txt', "r+") as file:
    content = file.read()
    file.seek(0, 0)
    file.write(str(graph.number_of_nodes())+ '\n' + content)
