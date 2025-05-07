#include <sstream>
#include <string.h>
#include <vector>
#include <utility>

#include "parsing_utils.hpp"

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

std::pair<std::vector<std::string>, bool> parse_by_qoute(std::string cmd_line){
    bool delim = false;
    char first;
    // char other;

    std::stringstream arg_buffer;
    std::string line = cmd_line + "\n";
    // int arg_cnt = 0;
    std::vector<std::string> pieces;

    // parse by 
    for (size_t i = 0; i < line.size(); i++){
        char ch = line[i];
        if (ch == '\'' || ch == '"'){
            if (!delim){
                first = ch;
                // other = '\'' + '"' - ch;
                delim = true;
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
            pieces.push_back(arg_buffer.str());
            arg_buffer.str(std::string());
            arg_buffer.clear();
        }
        else{
            arg_buffer << ch;
        }
    }
    if (delim){
        return { std::vector<std::string>(), false };
    }
    return { pieces, true };
}

int _parseCommandLine(const std::string& cmd_line, char** args, bool simple){
    FUNC_ENTRY();
    if (simple){
        int i = 0;
        std::istringstream iss(_trim(cmd_line));
        for (std::string s; iss >> s;){
            args[i] = (char*)malloc(s.length() + 1);
            memset(args[i], 0, s.length() + 1);
            strcpy(args[i], s.c_str());
            args[++i] = NULL;
        }
        FUNC_EXIT()
        return i;
    }

    auto parsed = parse_by_qoute(cmd_line);
    
    // // reparse the command
    // bool long_arg = false;
    // std::stringstream arg_buffer;
    // std::string line = cmd_line + "\n";
    // int arg_cnt = 0;

    // for (size_t i = 0; i < line.size(); i++){
    //     char ch = line[i];

    //     if (!long_arg){
    //         if (ch == '\''){
    //             long_arg = true;
    //         }
    //         else if (std::isspace(ch)){         // if starting a new word push
    //             if (!arg_buffer.str().empty()){
    //                 // put the new word in the buffer
    //                 std::string word = arg_buffer.str();
    //                 args[arg_cnt] = (char*)malloc(word.length() + 1);
    //                 memset(args[arg_cnt], 0, word.length() + 1);
    //                 strcpy(args[arg_cnt], word.c_str());
    //                 arg_cnt++;
    //                 args[arg_cnt] = NULL;

    //                 // reset buffer
    //                 arg_buffer.str(std::string());
    //                 arg_buffer.clear();
    //             }
    //             continue;
    //         }
    //         arg_buffer << ch;
    //     }
    //     else{
    //         arg_buffer << ch;
    //         if (ch == '\''){
    //             long_arg = false;
    //         }
    //     }
    // }
    int count = 0;
    for (auto& s : parsed.first){
        args[count] = (char*)malloc(s.length() + 1);
        memset(args[count], 0, s.length() + 1);
        strcpy(args[count], s.c_str());
        count++;
        args[count] = NULL;
    }

    FUNC_EXIT()
    return count;
}

bool _isBackgroundComamnd(const std::string& cmd_line){
    auto pos = cmd_line.find_last_not_of(WHITESPACE);
    if (pos == std::string::npos){
        return false;
    }
    return cmd_line[pos] == '&';
}

void _removeBackgroundSign(std::string& cmd_line){
    // find last character other than spaces
    unsigned int idx = cmd_line.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == std::string::npos){
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&'){
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[cmd_line.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}
