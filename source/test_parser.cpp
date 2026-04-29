#include <iostream>
#include <string>

using namespace std;

int main() {
    string json = R"([{"id":123,"game_time_to_beats":{"normally":123},"involved_companies":[{"company":{"name":"Valve"},"developer":true}],"name":"Half-Life","total_rating":95.5}])";
    
    size_t from = 0;
    auto start_obj = json.find('{', from);
    if (start_obj == string::npos) return 1;

    size_t end_obj = start_obj + 1;
    int brace_count = 1;
    bool in_str = false;
    while (end_obj < json.size() && brace_count > 0) {
        if (json[end_obj] == '"' && json[end_obj-1] != '\\') in_str = !in_str;
        if (!in_str) {
            if (json[end_obj] == '{') brace_count++;
            else if (json[end_obj] == '}') brace_count--;
        }
        end_obj++;
    }

    string obj = json.substr(start_obj, end_obj - start_obj);
    cout << "OBJ:\n" << obj << "\n\n";

    string root_obj = obj;
    int depth = 0;
    in_str = false;
    for (size_t i = 1; i + 1 < root_obj.size(); ++i) {
        if (root_obj[i] == '"' && root_obj[i-1] != '\\') in_str = !in_str;
        if (!in_str) {
            if (root_obj[i] == '{' || root_obj[i] == '[') {
                depth++;
                root_obj[i] = ' ';
            }
            else if (root_obj[i] == '}' || root_obj[i] == ']') {
                depth--;
                root_obj[i] = ' ';
            }
            else if (depth > 0) {
                root_obj[i] = ' ';
            }
        } else if (depth > 0) {
            root_obj[i] = ' ';
        }
    }

    cout << "ROOT OBJ:\n" << root_obj << "\n\n";
    return 0;
}
