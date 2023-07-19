import pprint
import queue
import networkx as nx
import matplotlib.pyplot as plt
import graphviz as gv

def main():
    tokens = []

    with open("ansi.c.grammer.y", "r") as f:
        lines = [x.strip() for x in f.readlines()]

        start = 0
        stop = 0

        for index, line in enumerate(lines):
            if "%token" in line:
                tokens.append(line)
            
            if "%%" in line:
                if start == 0:
                    start = index
                else:
                    stop = index
        
        grammar = lines[start + 1: stop]

    tokens = [y for x in tokens for y in x.split(' ')[1:]]

    print(tokens)

    adj_list: dict = {}  # tail to head

    grammar = [x for x in grammar if len(x) != 0]

    in_rule = False
    current_rule = ""

    for line in grammar:
        if in_rule:
            if line[0] == ':' or line[0] == '|':
                temp = line[2:]
                expression = temp.split(' ')
                temp_sequence = []
                for token in expression:
                    if "'" not in token:
                        temp_sequence.append(token)

                if current_rule in adj_list:
                    adj_list[current_rule] = adj_list[current_rule].union(set(temp_sequence))# .difference({current_rule}))
                else:
                    adj_list[current_rule] = set(temp_sequence)# .difference({current_rule})

            elif line[0] == ';':
                in_rule = False
                current_rule = ""
            else:
                print("doesnt happen")

        else:
            if len(line) != 0:
                in_rule = True
                current_rule = line
            else:
                continue


    head_to_tail: dict = {}
    # num_parents: dict = {}

    for tail, heads in adj_list.items():
        if tail not in head_to_tail:
            head_to_tail[tail] = []
        for head in heads:
            if head in head_to_tail:
                head_to_tail[head].append(tail)
            else:
                head_to_tail[head] = [tail]

    # pprint.pprint(head_to_tail)

    sorted_order = []

    q: queue.Queue = queue.Queue()

    for head in head_to_tail:
        if len(head_to_tail[head]) == 0:
            q.put(head)

    while not q.empty():
        cur = q.get()

        sorted_order.append(cur)

        for head in adj_list[cur]:
            head_to_tail[head].remove(cur)

            if len(head_to_tail[head]) == 0:
                q.put(head)

    if not all([len(x) == 0 for x in head_to_tail.values()]):
        print("not acyclic")
        print(sorted_order)
    else:
        print(sorted_order)


    acyclic_adj_list: dict = {}

    q: queue.Queue = queue.Queue()

    q.put('translation_unit')

    while not q.empty():
        cur = q.get()
        if cur not in acyclic_adj_list:
            acyclic_adj_list[cur] = []

        if cur not in adj_list:
            continue

        for head in adj_list[cur]:
            if head not in acyclic_adj_list:
                acyclic_adj_list[cur].append(head)
                q.put(head)

    print("acyclic version")
    pprint.pprint(acyclic_adj_list)


    g = gv.Digraph()


    for tail, heads in adj_list.items():
        for head in heads:
            g.edge(tail, head)

    g.render("dag.png")

    # G = nx.DiGraph()
    #
    # for tail, heads in acyclic_adj_list.items():
    #     G.add_edges_from([(tail, head) for head in heads])
    #
    #
    # pos = nx.spring_layout(G)
    # nx.draw_networkx_nodes(G, pos)
    # nx.draw_networkx_labels(G, pos)
    # nx.draw_networkx_edges(G, pos)
    # plt.show()



    # G = nx.DiGraph()
    #
    # for tail, heads in adj_list.items():
    #     G.add_edges_from([(tail, head) for head in heads])
    #
    # pos = nx.spring_layout(G)
    #
    # nx.draw_networkx_nodes(G, pos)
    # nx.draw_networkx_labels(G, pos)
    # nx.draw_networkx_edges(G, pos)
    # plt.show()


if __name__ == "__main__":
    main()
