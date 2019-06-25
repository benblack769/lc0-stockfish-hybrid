import json
import sys
import random

assert len(sys.argv) == 3,"need cutoff size, graph filename"
cutoff = int(sys.argv[1])
filename = sys.argv[2]

graph_data = json.load(open(filename))

remove_nodes = set()

for item in graph_data:
    if item['type'] == "node":
        id = item['node']
        if item['node_count'] < cutoff:# or (item['node_count'] < 5 and random.random() > 0.1):
            remove_nodes.add(id)

for item in graph_data:
    if item['type'] == "edge":
        parent = item['parent']
        child = item['child']
        if  parent in remove_nodes or child in remove_nodes:
            remove_nodes.add(child)

print("digraph G{")
for item in graph_data:
    if item['type'] == "node":
        shape = "box" if item['type'] == "black" else "oval"
        id = item['node']
        label = item['node_count']
        if id in remove_nodes:
            continue

        print('\t{id} [ shape={shape}, label="{label}" ]'.format(id=id,shape=shape,label=label))

for item in graph_data:
    if item['type'] == "edge":
        color = "green" if item['is_allowed'] else "red"
        parent = item['parent']
        child = item['child']
        if parent in remove_nodes or child in remove_nodes:
            continue

        name = item['name']
        print('\t{parent} -> {child} [ color={color}, label="{name}" ]'.format(parent=parent,child=child,color=color,name=name))

print("}")
