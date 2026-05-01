#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "utils.hpp"
#include "citation.h"


std::vector<Citation*> loadCitations(const std::string& filename) {
    // FIXME: load citations from file
    std::string id;
    std::string type;
    std::ifstream file{ filename };
    std::vector<Citation*> citations;
    if (!file.is_open()){
        std::exit(1);
    }
    nlohmann::json data = nlohmann::json::parse(file);
    for (int i{ 0 }; i < (data["citations"].size()); i++) {
        if (!data["citations"][i].contains("id") || !data["citations"][i].contains("type")) {
            std::exit(1);
        }
        if (!data["citations"][i]["id"].is_string() || !data["citations"][i]["type"].is_string()) {
            std::exit(1);
        }
        id = data["citations"][i]["id"];
        type = data["citations"][i]["type"];
        //check id
        for (char c : id) {
            if (!(('0' <= c && c <= '9') ||
                ('A' <= c && c <= 'Z') ||
                ('a' <= c && c <= 'z'))) {
                std::exit(1);
            }
        }
        //check/define type
        if (type == "book") {
            if (!data["citations"][i].contains("isbn") || !data["citations"][i]["isbn"].is_string()) {
                std::exit(1);
            }
            Book* tmptr = new Book(id, data["citations"][i]["isbn"]);
            tmptr->search();
            citations.push_back(tmptr);  
        }
        else if (type == "webpage") {
            if (!data["citations"][i].contains("url") || !data["citations"][i]["url"].is_string()) {
                std::exit(1);
            }
            Webpage* tmptr = new Webpage(id, data["citations"][i]["url"]);
            tmptr->search();
            citations.push_back(tmptr);
        }
        else if (type == "article") {
            if (!data["citations"][i].contains("title") || !data["citations"][i]["title"].is_string() ||
                !data["citations"][i].contains("author") || !data["citations"][i]["author"].is_string() ||
                !data["citations"][i].contains("journal") || !data["citations"][i]["journal"].is_string() ||
                !data["citations"][i].contains("year") || !data["citations"][i]["year"].is_number_integer() ||
                !data["citations"][i].contains("volume") || !data["citations"][i]["volume"].is_number_integer() ||
                !data["citations"][i].contains("issue") || !data["citations"][i]["issue"].is_number_integer()) {
                std::exit(1);
            }
            citations.push_back(new Article(id, data["citations"][i]["title"], data["citations"][i]["author"], 
                data["citations"][i]["journal"], data["citations"][i]["year"].get<int>(), 
                data["citations"][i]["volume"].get<int>(), data["citations"][i]["issue"].get<int>()));
        }
        else {
            std::exit(1);
        }
    }
    return citations;
}

void readArg(int argc, char** argv, std::string &citation_path, std::string &output_path, std::string &input_path) {
    bool cFlag = false;
    bool oFlag = false;
    bool iFlag = false;
    for (int i = 1; i <= argc - 1; i++) {
        std::string arg = argv[i];
        std::string argNext;
        if (arg == "-c") {
            if (!(cFlag)) {
                if ((i + 1) < argc) {
                    if (argv[i + 1][0] == '-') {
                        std::exit(1);
                    }
                    else {
                        argNext = argv[i + 1];
                        citation_path = argNext;
                        cFlag = true;
                        i += 1;
                    }
                }
                else {
                    std::exit(1);
                }
            }
            else {
                std::exit(1);
            }
        }

        else if (arg == "-o") {
            if (!(oFlag)) {
                if ((i + 1) < argc) {
                    if (argv[i + 1][0] == '-') {
                        std::exit(1);
                    }
                    else {
                        argNext = argv[i + 1];
                        output_path = argNext;
                        oFlag = true;
                        i += 1;
                    }
                }
                else {
                    std::exit(1);
                }
            }
            else {
                std::exit(1);
            }
        }

        else if (arg == "-") {
            if(!(iFlag)) {
                input_path = "-";
                iFlag = true;
            }
            else { std::exit(1); }
        }
        else if (argv[i][0] != '-') {
            if (!(iFlag)) {
                input_path = arg;
                iFlag = true;
            }
            else { std::exit(1); }
        }

        else {
            std::exit(1);
        }
    }
    if (!(cFlag)) {
        std::exit(1);
    }
    
    if ((input_path.empty())) {
        std::exit(1);
    }
}

int main(int argc, char** argv) {
    // "docman", "-c", "citations.json", "input.txt"
    std::string citation_path{};
    std::string output_path{};
    std::string input_path{};
    readArg(argc, argv, citation_path, output_path, input_path);

    //input
    std::string input;
    std::string line;
    if (input_path == "-") {
        while (std::getline(std::cin, line)) {
            input += line + '\n';
        }
    }
    else {
        input = readFromFile(input_path);
    }
    
    //citation
    auto citations = loadCitations(citation_path);
    std::vector<Citation*> printedCitations{};

    //output(in case the output is constructed)
    std::ofstream outFile;
    std::ostream* output = &std::cout;
    if (!(output_path.empty())) {
        outFile.open(output_path);
        if (!outFile.is_open()) {
            std::exit(1);
        }
        else {
            output = &outFile;
        }
    }
    
    // FIXME: read all input to the string(done up), todo: and process citations in the input text(damn I can't be more robust)
    for (size_t i = 0; i < input.size(); i++) {
        std::string idToBe;
        if (input[i] == '[') {
            bool closed = false;
            for (size_t j = i + 1; j < input.size(); j++) {
                if (input[j] != ']') {
                    idToBe.push_back(input[j]);
                }
                else {
                    closed = true;
                    i = j;
                    if (idToBe == "") { std::exit(1); }
                    bool found = false;
                    for (auto x : citations) {
                        if (idToBe == x->id) {
                            bool isIn = false;
                            for (auto y : printedCitations) {
                                if (y->id == idToBe) {
                                    isIn = true;
                                    break;
                                }
                            }
                            if (!isIn) {
                                printedCitations.push_back(x);
                            }
                            found = true;
                            break;
                        }
                        
                    }
                    if (!found) {
                        std::exit(1);
                    }
                    break;
                }
            }
            if (!closed) {
                std::exit(1);
            }
        }
        else if (input[i] == ']') {
            std::exit(1);
        }

    }

    std::sort(printedCitations.begin(), printedCitations.end(), [](Citation* A, Citation* B) {return A->id < B->id;});

    *output << input;  // print the paragraph first
    if (!input.empty() && input.back() != '\n') {
        *output << '\n';
    }
    *output << "\n\nReferences:\n";
    
    for (auto c : printedCitations) {
        // FIXME: print citation
        c->print(*output);
    }

    for (auto c : citations) {
        delete c;
    }
}
