#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <stack>

using namespace std;

class Vertex
{
    public:
        int val;
        Vertex(int v): val(v) {};
};

class GraphAdjList {
public:
    unordered_map<Vertex *, vector<Vertex *>> adjList;

    void remove(vector<Vertex *> &vec, Vertex *vet) {
        for (int i = 0; i < vec.size(); i++) {
            if (vec[i] == vet) {
                vec.erase(vec.begin() + i);
                break;
            }
        }
    }

    GraphAdjList(const vector<vector<Vertex *>> &edges) {
        // 添加所有顶点和边
        for (const vector<Vertex *> &edge : edges) {
            addVertex(edge[0]);
            addVertex(edge[1]);
            addEdge(edge[0], edge[1]);
        }
    }

    int size() {
        return adjList.size();
    }

    void addEdge(Vertex *vet1, Vertex *vet2) {
        if (!adjList.count(vet1) || !adjList.count(vet2) || vet1 == vet2) // 没检查有没有存在，上面删除也没全删除
            throw invalid_argument("no vertex");

        adjList[vet1].push_back(vet2);
        adjList[vet2].push_back(vet1);
    }

    void removeEdge(Vertex *vet1, Vertex *vet2) {
        if (!adjList.count(vet1) || !adjList.count(vet2) || vet1 == vet2)
            throw invalid_argument("no vertex");
        // 删除边 vet1 - vet2
        remove(adjList[vet1], vet2);
        remove(adjList[vet2], vet1);
    }

    void addVertex(Vertex *vet) {
        if (adjList.count(vet))
            return;

        adjList[vet] = vector<Vertex *>();
    }

    void removeVertex(Vertex *vet) {
        if (!adjList.count(vet))
            throw invalid_argument("no vertex");

        adjList.erase(vet);

        for (auto &adj : adjList) {
            remove(adj.second, vet);
        }
    }

    void print() {
        cout << "Adjancy List is =" << endl;
        for (auto &adj : adjList) {
            const auto &key = adj.first;
            const auto &vec = adj.second;
            cout << key->val << " : ";
            printVector(vec);
        }
    }

    void printVector(const vector<Vertex *> vec)
    {
        for (const auto &v : vec)
        {
            cout << v->val << " | ";
        }
        cout << endl;
    }
};

vector<Vertex *> bfs_traversal(GraphAdjList& graph_list, Vertex* start_vertex);
vector<Vertex *> dfs_traversal(GraphAdjList& graph_list, Vertex* start_vertex);
void printVector(const vector<Vertex *> vec);

int main()
{
    vector<vector<Vertex *>> data;
    Vertex v1(1);
    Vertex v2(2);
    Vertex v3(3);
    Vertex v4(4);
    Vertex v5(5);
    Vertex v6(6);

    vector<Vertex *> vec1 = {&v1, &v2};
    vector<Vertex *> vec2 = {&v1, &v3};
    vector<Vertex *> vec3 = {&v3, &v4};
    vector<Vertex *> vec4 = {&v2, &v5};
    vector<Vertex *> vec5 = {&v5, &v6};

    data.push_back(vec2);
    data.push_back(vec1);
    data.push_back(vec3);
    data.push_back(vec4);
    data.push_back(vec5);

    GraphAdjList g(data);

    g.print();

    cout << "BFS search: " << endl;
    auto bfs_order = bfs_traversal(g, &v1);
    printVector(bfs_order);

    cout << "DFS search: " << endl;
    auto dfs_order = dfs_traversal(g, &v1);
    printVector(dfs_order);

    // g.removeVertex(&v1);
    g.remove(g.adjList[&v1], &v2);
    // g.remove(g.adjList[&v2], &v1); // if we do not do this then it becomes a directed graph

    cout << &vec1 << endl; // They are different
    cout << &g.adjList[&v1] << endl;

    cout << "we delete v2 in v1: " << endl;
    g.print();
}

vector<Vertex *> bfs_traversal(GraphAdjList& graph_list, Vertex* start_vertex)
{
    queue<Vertex *> vertex_queue;
    vertex_queue.push(start_vertex);
    vector<Vertex *> output;
    unordered_set<Vertex *> visited = {start_vertex};

    while (!vertex_queue.empty())
    {
        Vertex* v = vertex_queue.front();
        vertex_queue.pop();
        output.push_back(v);

        for (const auto & item : graph_list.adjList[v])
        {
            if (!visited.count(item))
            {
                vertex_queue.push(item);
                visited.emplace(item);
            }
        }
    }

    return output;
}

vector<Vertex *> dfs_traversal(GraphAdjList& graph_list, Vertex* start_vertex)
{
    stack<Vertex *> vertex_stack;
    vertex_stack.push(start_vertex);
    vector<Vertex *> output;
    unordered_set<Vertex *> visited = {start_vertex};

    while (!vertex_stack.empty())
    {
        Vertex* v = vertex_stack.top();
        vertex_stack.pop();
        output.push_back(v);

        for (const auto & item : graph_list.adjList[v])
        {
            if (!visited.count(item))
            {
                vertex_stack.push(item);
                visited.emplace(item);
            }
        }
    }

    return output;
}

void printVector(const vector<Vertex *> vec)
{
    for (const auto &v : vec)
    {
        cout << v->val << " | ";
    }
    cout << endl;
}