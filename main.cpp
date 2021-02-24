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
#include <peglib.h>

static std::set<std::string> filter_safe{ // NOLINT(cert-err58-cpp)
        "BlockEnd",
        "BlockStart",
        "LP",
        "RP",
        "Semicolon",
        "SequenceSign",
        "eof"
};

static std::set<std::string> filter_if_empty{ // NOLINT(cert-err58-cpp)
        "OptionalEllipsis"
};

static std::set<std::string> filter_unsafe{ // NOLINT(cert-err58-cpp)
        "Arrow",
        "BitOr",
        "Colon",
        "Comma",
        "Eq",
        "GT",
        "LSqB",
        "LT",
        "RSqB",
};

static std::set<std::string> unsafe_parents{ // NOLINT(cert-err58-cpp)
        "AndType",
        "ApiRefs",
        "LimitedTypeName",
        "ClassFields",
        "ArrayConstruction",
        "ExpressionList",
        "Comprehension",
        "ComprPipeline",
        "Constant",
        "ImportList",
        "MatchCase",
        "MethodExpression",
        "MultiparamLambda",
        "NamedAssignment",
        "NamedAssignmentList",
        "NamedRange",
        "OptionalEllipsis",
        "OrType",
        "ParallelAssignment",
        "SingleParamLambda",
        "TupleConstruction",
        "TupleDecl",
        "Type",
        "TypeArguments",
        "TypedArgList",
        "TypeNameList",
        "ValueReference"
};

template<typename T>
std::shared_ptr<T> optimize(bool optimize_unsafe,
                            std::shared_ptr<T> original,
                            std::shared_ptr<T> parent = nullptr) {

    auto ast = std::make_shared<T>(*original);
    ast->parent = parent;
    ast->nodes.clear();
    for (const auto &node : original->nodes) {
        if (filter_safe.find(node->name) != filter_safe.end()) {
            continue;
        }

        if (filter_if_empty.find(node->name) != filter_if_empty.end() && node->nodes.empty()) {
            continue;
        }

        if (optimize_unsafe && filter_unsafe.find(node->name) != filter_unsafe.end()) {
            continue;
        }

        auto child = optimize(unsafe_parents.find(node->name) != unsafe_parents.end(), node, ast);

        ast->nodes.push_back(child);
    }
    return ast;
}

template<typename T>
void walk_ast(const std::shared_ptr<T> &node, std::function<void(const std::shared_ptr<T> &)> fn) {
    fn(node);

    for (const auto &child : node->nodes) {
        walk_ast(child, fn);
    }
}

template<typename T>
void walk_ast(const std::shared_ptr<T> &node, std::function<void(const std::shared_ptr<T> &, int)> fn, int depth) {
    fn(node, depth);

    for (const auto &child : node->nodes) {
        walk_ast(child, fn, depth + 1);
    }
}

void print_tree(const std::shared_ptr<peg::Ast> &node, int depth) {
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }

    auto name = node->name == node->original_name ? node->name : node->original_name + "/" + node->name;
    auto prefix = (node->nodes.size() > 0 ? "+ " : "- ");

    std::cout << prefix << name << " (" << node->token << ")" << std::endl;
}

extern std::string grammarText;

int usage() {
    std::cout << "Usage: cbc [-v] [-b] file1.ext ... fileN.ext" << std::endl;
    return -100;
}

peg::parser load_parser(bool bench) {
    peg::parser peg;

    auto p0 = std::chrono::high_resolution_clock::now();

    peg.log = [](size_t ln, size_t col, const std::string &msg) mutable {
        std::cout << "Error in grammar at " << std::to_string(ln) << ":" << std::to_string(col) << "::" << msg
                  << std::endl;
    };

    auto ret = peg.load_grammar(grammarText.c_str(), grammarText.size());
    auto p1 = std::chrono::high_resolution_clock::now();

    if (bench) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(p1 - p0);
        std::cout << "Loading grammar " << duration.count() << "us" << std::endl;
    }

    if (!ret) {
        std::cout << "Unable to proceed, exiting" << std::endl;
        exit(-200);
    }

    peg.enable_ast();
    peg.enable_packrat_parsing();

    return peg;
}

std::shared_ptr<peg::Ast>
parse_input(peg::parser peg, const std::string &codeText, const std::string &fileName, bool bench) {
    std::string codeErrors = fileName + "\n";

    auto p1 = std::chrono::high_resolution_clock::now();

    peg.log = [&](size_t ln, size_t col, const std::string &msg) mutable {
        std::cout << fileName << " error at " << std::to_string(ln) << ":" << std::to_string(col) << "::" << msg
                  << std::endl;
    };

    std::shared_ptr<peg::Ast> ast;
    auto ret = peg.parse(codeText, ast, fileName.data());
    auto p2 = std::chrono::high_resolution_clock::now();

    if (bench) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(p2 - p1);
        std::cout << fileName << " parsing " << duration.count() << "us" << std::endl;
    }

    if (ret && ast) {
        auto optimized = optimize(true, peg.optimize_ast(ast));
        auto p3 = std::chrono::high_resolution_clock::now();

        if (bench) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(p3 - p2);
            std::cout << fileName << " AST optimization " << duration.count() << "us" << std::endl;
        }

        return optimized;
    }

    return ast;
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
