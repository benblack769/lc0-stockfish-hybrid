import json
import sys
import random
import math

assert len(sys.argv) == 3,"need cutoff size, graph filename"
cutoff = int(sys.argv[1])
filename = sys.argv[2]

graph_data = json.load(open(filename))

remove_nodes = set()

all_lines = []

for item in graph_data:
    if item['type'] == "node":
        id = item['node']
        if item['node_count'] < cutoff:# or (item['node_count'] < 5 and random.random() > 0.1):
            remove_nodes.add(id)
        else:
            all_lines.append(item)

for item in graph_data:
    if item['type'] == "edge":
        parent = item['parent']
        child = item['child']
        if  parent in remove_nodes or child in remove_nodes:
            remove_nodes.add(child)
        else:
            all_lines.append(item)

print(json.dumps(all_lines))
