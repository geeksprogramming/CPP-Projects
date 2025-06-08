/**
 * @file schedule.cpp
 * Exam scheduling using graph coloring
 */

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <set>

#include "schedule.h"
#include "utils.h"
#include <algorithm>

using namespace std;


// We can assume that there will always be an entry after each comma
// and a comma between every two entries.
// So what if file is empty or contains empty lines ? split_string will return empty strings.
// i.e., split_string("", sep, fields) = {""}
// i.e., split_string("\n\n\n", sep, fields) = {"", "", ""}
// We want to prevent that. We want to return empty vector in these cases.
// We want to return non-empty fields only.
// This function strips spaces, so sep must not be a space.
int my_split_string(const std::string & str1, char sep, std::vector<std::string> &fields) {
    std::string tmp = trim(str1);
    if (!tmp.empty()) {
        return split_string(tmp, sep, fields);
    }
    return 0;
}

/**
 * Given a filename to a CSV-formatted text file, create a 2D vector of strings where each row
 * in the text file is a row in the V2D and each comma-separated value is stripped of whitespace
 * and stored as its own string. 
 * 
 * Your V2D should match the exact structure of the input file -- so the first row, first column
 * in the original file should be the first row, first column of the V2D.
 *  
 * @param filename The filename of a CSV-formatted text file. 
 */
V2D file_to_V2D(const std::string & filename){
    V2D csv;

    std::string file = file_to_string(filename);

    std::vector <std::string> rows;
    int nRows = my_split_string(file, '\n', rows);

    for (const std::string &row : rows) {
        std::vector <std::string> v;
        int nFields = my_split_string(row, ',', v);

        if (nFields) {
            for (std::string &field : v) {
                field = trim(field);
            }
            csv.push_back(v);
        }
    }

    return csv;
}

/**
 * Given a course roster and a list of students and their courses, 
 * perform data correction and return a course roster of valid students (and only non-empty courses).
 * 
 * A 'valid student' is a student who is both in the course roster and the student's own listing contains the course
 * A course which has no students (or all students have been removed for not being valid) should be removed
 * 
 * @param cv A 2D vector of strings where each row is a course ID followed by the students in the course
 * @param student A 2D vector of strings where each row is a student ID followed by the courses they are taking
 */
V2D clean(const V2D & cv, const V2D & student){

    V2D corrected;

    for (const std::vector<std::string> &row : cv) {
        if (!row.empty()) {
            std::vector <std::string> correctedRow;            
            std::string course = row[0];
            correctedRow.push_back(course);
            // For each student in the course
            for (int i = 1; i < (int)row.size(); ++i) {
                std::string studentName = row[i];
                bool found = false;
                // Find the student 
                for (const vector<string> &studentRow : student) {
                    if (!studentRow.empty() && studentRow.front() == studentName) {
                        // Find if the student contains the course
                        if (find(studentRow.begin() + 1, studentRow.end(), course) != studentRow.end()) {
                            found = true;
                            break;
                        }
                    }
                }
                if (found) {
                    correctedRow.push_back(studentName);
                }
            }
            if (correctedRow.size() > 1) {
                corrected.push_back(correctedRow);
            }
        }
    }

    return corrected;
}


class Graph {
public:
    Graph() {}

    void addVertex(string const& v) {
        graph[v] = vector<string>();
        verts.push_back(v);
    }

    void addUndirectedEdge(string const& u, string const &v) {
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    vector <string> vertices() {
        return verts;
    }

    vector <string> edges(string const& u) {
        return graph[u];
    }

private:
    map <string, vector<string>> graph;
    vector <string> verts;
};


Graph buildGraph(const V2D &courses) {
    Graph graph;    

    // Create Vertices
    // Each course name is a vertex in a graph
    for (auto &row : courses) {
        if (!row.empty()) {
            string const &coursename = row[0];
            graph.addVertex(coursename);
        }
    }

    // Create edges
    // If two courses share atleast one student, then there is an 
    // undirected edge between them.
    for (const auto& rowA : courses) {
        for (const auto& rowB : courses) {
            
            if (rowA.empty() || rowB.empty()) {
                continue;
            }

            // same row, so skip it.
            // (or) Different row but same coursename (should not happen)
            // skip it. 
            string const &courseName1 = rowA[0];
            string const &courseName2 = rowB[0];
            if (courseName1 == courseName2) {
                continue;
            }

            // Check if a student from courseName1 is also
            // present in courseName2. If it is then there is an undirected
            // edge between them.
            for (int i = 1; i < (int) rowA.size(); ++i) {
                const string &student = rowA[i];
                if (find(rowB.begin() + 1, rowB.end(), student) != rowB.end()) {
                    graph.addUndirectedEdge(courseName1, courseName2);
                    break;
                }
            }
        }
    }

    return graph;
}

/*
    Perform Depth first search starting from node.
    Find a suitable color for it which is different for its adjacent neighbours.
    If a color is not found for any node in the dfs path then return false.
    Otherwerise return true if a color is found for every node in the path.
*/
bool dfs(Graph &graph,
            string const& node, 
            map<string, bool> &visited, 
            map<string, string> &colors,
            const std::vector<std::string> &timeslots) {
        
    if (visited[node]) {
        return true;
    }

    visited[node] = true;

    vector <string> edges = graph.edges(node);

    // We need to find a color which is not used by any adjacent nodes.
    // 1. Add all the colors used by adjacents nodes into usedColors set.
    set <string> usedColors;

    for (string const &u : edges) {
        string const &color = colors[u];
        if (!color.empty()) {
            usedColors.insert(color);
        }
    }

    // Find a color for this node from timeslots
    // which doesn't exists in usedColors.
    bool found = false;
    for (const string &color : timeslots) {
        if (usedColors.find(color) == usedColors.end()) {
            found = true;
            colors[node] = color;
            break;
        }
    }

    if (!found) {
        return false;
    }

    // Now find colors for adjacent nodes
    for (const string & v : edges) {
        if (!dfs(graph, v, visited, colors, timeslots)) {
            return false;
        }
    }

    return true;
}

bool solve(Graph &graph, const std::vector<std::string> &timeslots, 
                            string const &startNode, V2D &answer) {

    map <string, bool> visited;
    map <string, string> colors;

    vector <string> vertices = graph.vertices();
    vertices.insert(vertices.begin(), startNode);

    for (const string &u : vertices) {
        if (!dfs(graph, u, visited, colors, timeslots)) {
            return false;
        }
    }

    map <string, set<string>> tmp;
    for (const string & color : timeslots) {
        tmp[color] = set<string>();
    }
    for (const auto & kv : colors) {
        string const & course = kv.first;
        string const & color = kv.second;
        tmp[color].insert(course);
    }

    answer.clear();

    for (const auto & kv : tmp) {
        vector <string> row;
        row.push_back(kv.first);
        for (const string & course : kv.second) {
            row.push_back(course);
        }
        answer.push_back(row);
    }

    return true;
}

/**
 * Given a collection of courses and a list of available times, create a valid scheduling (if possible).
 * 
 * A 'valid schedule' should assign each course to a timeslot in such a way that there are no conflicts for exams
 * In other words, two courses who share a student should not share an exam time.
 * Your solution should try to minimize the total number of timeslots but should not exceed the timeslots given.
 * 
 * The output V2D should have one row for each timeslot, even if that timeslot is not used.
 * 
 * As the problem is NP-complete, your first scheduling might not result in a valid match. Your solution should 
 * continue to attempt different schedulings until 1) a valid scheduling is found or 2) you have exhausted all possible
 * starting positions. If no match is possible, return a V2D with one row with the string '-1' as the only value. 
 * 
 * @param courses A 2D vector of strings where each row is a course ID followed by the students in the course
 * @param timeslots A vector of strings giving the total number of unique timeslots
 */
V2D schedule(const V2D &courses, const std::vector<std::string> &timeslots){

    Graph graph = buildGraph(courses);

    // Part3
    // Try every vertex as a potential start node
    for (string const &startNode : graph.vertices()) {
        V2D answer;
        if (solve(graph, timeslots, startNode, answer)) {
            return answer;
        }
    }

    V2D notfound;
    notfound.push_back({"-1"});
    return notfound;
}
