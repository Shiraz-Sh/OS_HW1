#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include <algorithm>
#include <cctype>


const std::string WHITESPACE = " \n\r\t\f\v";
using namespace std;

std::string s = R"(alias t2='sleep'> tmp.txt)";

std::string _ltrim(const std::string& s){
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string _rtrim(const std::string& s){
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string _trim(const std::string& s){
    return _rtrim(_ltrim(s));
}

std::vector<std::pair<std::string, bool>> parse_by_qoute(std::string cmd_line){
    bool delim = false;
    bool delim_seen_in_word = false;
    char first;

    std::stringstream arg_buffer;
    std::string line = cmd_line + "\n";
    std::vector<std::pair<std::string, bool>> pieces;

    // parse by
    for (size_t i = 0; i < line.size(); i++){
        char ch = line[i];
        if (ch == '\'' || ch == '"'){
            if (!delim){
                first = ch;
                delim = true;
                delim_seen_in_word = true;
            }
            else{
                if (ch == first){
                    delim = false;
                }
                else{
                    arg_buffer << ch;
                }
            }
        }
        else if (!delim && isspace(ch)){
            pieces.push_back({ arg_buffer.str(), delim_seen_in_word });
            arg_buffer.str(std::string());
            arg_buffer.clear();
            delim_seen_in_word = false;
        }
        else{
            arg_buffer << ch;
        }
    }
    if (delim){
        return std::vector<std::pair<std::string, bool>>();
    }
    return pieces;
}

std::vector<std::string> parse_by_delim(std::string cmd_line, std::string delim){
    std::vector<std::string> pieces;

    auto pos = cmd_line.find(delim);
    while (pos != std::string::npos){
        // push
        pieces.push_back(_trim(cmd_line.substr(0, pos)));
        pieces.push_back(delim);

        // update
        cmd_line = cmd_line.substr(pos + delim.size());
        pos = cmd_line.find(delim);
    }

    if (!cmd_line.empty())
        pieces.push_back(_trim(cmd_line));
    return pieces;
}

std::pair<std::string, std::string> split_by_first_token(const std::vector<std::string>& parts, const std::string& token){
    std::string lhs, rhs;
    bool found = false;

    for (const auto& part : parts){
        if (!found && part.compare(token) == 0){
            found = true;
            continue; // skip the token itself
        }

        if (!found)
            lhs += part + " ";
        else
            rhs += part + " ";
    }

    // Trim trailing space from both sides (optional)
    auto trim_end = [](std::string s){
        if (!s.empty() && s.back() == ' ') s.pop_back();
        return s;
    };

    return { trim_end(lhs), trim_end(rhs) };
}

std::vector<std::string> clean(const std::vector<string>& s_vec){
    vector<string> temp;
    for (auto& s : s_vec){
        if (std::all_of(s.begin(), s.end(), [](unsigned char c){ return std::isspace(c); })){
            continue;
        }
        temp.push_back(s);
    }
    return temp;
}

int main(){
    auto args = parse_by_qoute(s);
    std::vector<string> concat;

    for (auto& p : args){
        if (p.second){
            concat.push_back(p.first);
        }
        else{
            auto res = parse_by_delim(p.first, "|");
            std::vector<string> temp_concat;
            for (auto& str : res){
                auto res2 = parse_by_delim(p.first, ">");
                temp_concat.insert(temp_concat.end(), res2.begin(), res2.end());
            }
            concat.insert(concat.end(), temp_concat.begin(), temp_concat.end());
        }
    }

    concat = clean(concat);

    for (auto& v : concat){
        cout << "[ " << v << " ] ";
    }

    // auto [left, right] = split_by_first_token(concat, ">");
    // if (right.empty()){
    //     auto [left2, right2] = split_by_first_token(concat, "|");
    // }



    cout << endl;
}