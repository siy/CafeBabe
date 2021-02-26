//
//  main.cpp
//
//  Copyright (c) 2020 Sergiy Yevtushenko. All rights reserved.
//  MIT License
//

#include <functional>
#include <fstream>
#include <sstream>
#include <set>
#include <chrono>
#include "cafebabe.h"

void print_tree(const std::shared_ptr<peg::Ast> &node, int depth) {
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }

    auto name = node->name == node->original_name ? node->name : node->original_name + "/" + node->name;
    auto prefix = (node->nodes.size() > 0 ? "+ " : "- ");

    std::cout << prefix << name << " (" << node->token << ")" << std::endl;
}

int usage() {
    std::cout << "Usage: cbc [-v] [-b] file1.ext ... fileN.ext" << std::endl;
    return -100;
}

int main(int argc, const char **argv) {
    if (argc <= 1) {
        return usage();
    }

    int pos = 1;
    bool verbose = false;
    bool bench = false;

    while (pos < argc && argv[pos][0] == '-') {
        if (argv[pos][1] == 0 || argv[pos][2] != 0) {
            std::cout << "Unknown option \"" << argv[pos] << "\"" << std::endl;
            return -2;
        }

        switch (argv[pos][1]) {
            case 'v':
                verbose = true;
                break;
            case 'b':
                bench = true;
                break;
        }

        pos++;
    }

    if (pos >= argc) {
        return usage();
    }

    auto peg = load_parser(bench);
    auto rc = true;

    while (pos < argc) {
        std::string fileName = argv[pos];
        std::ifstream ifs2(fileName);

        if (!ifs2.is_open()) {
            std::cout << fileName << " MISSING" << std::endl;
            return -2;
        }

        auto start = std::chrono::high_resolution_clock::now();

        std::string code((std::istreambuf_iterator<char>(ifs2)),
                         (std::istreambuf_iterator<char>()));

        auto ast = parse_input(peg, code, fileName, bench);
        auto stop = std::chrono::high_resolution_clock::now();

        if (ast) {
            if (verbose) {
                std::function<void(const std::shared_ptr<peg::Ast> &, int)> fn = print_tree;
                walk_ast(ast, fn, 0);
            }
        }

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        if (bench) {
            std::cout << fileName << (ast ? " PASS" : " FAIL") << " in " << duration.count() << "us" << std::endl
                      << std::endl;
        } else {
            std::cout << fileName << (ast ? " PASS" : " FAIL") << std::endl;
        }

        rc = rc && ast;
        pos++;
    }

    return rc ? 0 : -1;
}
