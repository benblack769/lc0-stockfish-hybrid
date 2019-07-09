import json
import sys
import random
import math

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
min_width = math.log(cutoff)
for item in graph_data:
    if item['type'] == "node":
        shape = "oval"#"box" if item['type'] == "black" else "oval"
        id = item['node']
        color = "red" if item['turn'] == "black" else "blue"
        label = str(item['q_val']) #+ "--" + "{:,}".format(item['ab_time'])#""#item['node_count']
        if id in remove_nodes:
            continue

        width = (math.log(item['node_count']) - min_width + 1)#/20

        print('\t{id} [ shape={shape}, label="{label}" penwidth={width} ]'.format(id=id,shape=shape,label=label,width=width))

for item in graph_data:
    if item['type'] == "edge":
        color = "green" if item['is_allowed'] else "red"
        parent = item['parent']
        child = item['child']
        if parent in remove_nodes or child in remove_nodes:
            continue

        name = item['name']
        print('\t{parent} -> {child} [ color={color}, label="{name}" arrowhead="none" ]'.format(parent=parent,child=child,color=color,name=name))

print("}")
