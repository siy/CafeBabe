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

std::function<void(size_t, size_t, const std::string &)> makeErrorLogger(std::string &errorText) {
    return [&](size_t ln, size_t col, const std::string &msg) mutable {
        errorText += "  at " + std::to_string(ln) + ":" + std::to_string(col) + "::" + msg + "\n";
    };
}

bool parse_grammar(const std::string &text, peg::parser &peg, std::string &errorText) {
    peg.log = makeErrorLogger(errorText);
    return peg.load_grammar(text.c_str(), text.size());
}

bool parse_code(const std::string &text, peg::parser &peg, std::string &errorText, std::shared_ptr<peg::Ast> &ast) {
    peg.log = makeErrorLogger(errorText);
    return peg.parse_n(text.data(), text.size(), ast);
}

struct lint_result {
    std::string grammarErrors;
    std::string codeErrors;
    std::string astOptimized;
};

int usage();

static std::set<std::string> filter_safe{ // NOLINT(cert-err58-cpp)
        "__",
        "BlockEnd",
        "BlockStart",
        "LP",
        "RP",
        "Semicolon",
        "SequenceSign"
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
        "ArrayConstruction",
        "ArrayElements",
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
        "Type",
        "TypeArguments",
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
    for (auto node : original->nodes) {
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

extern std::string grammarText;

peg::parser load_parser(bool bench) {
    std::string grammarErrors;
    peg::parser peg;

    auto p0 = std::chrono::high_resolution_clock::now();
    auto ret = parse_grammar(grammarText, peg, grammarErrors);
    auto p1 = std::chrono::high_resolution_clock::now();

    if (bench) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(p1 - p0);
        std::cout << "Loading grammar " << duration.count() << "us" << std::endl;
    }

    if (!ret) {
        std::cout << "Error loading grammar: " << std::endl << grammarErrors << std::endl;
    }
    peg.enable_ast();
    peg.enable_packrat_parsing();

    return peg;
}

int usage() {
    std::cout << "Usage: cbc [-v] [-b] file1.ext ... fileN.ext" << std::endl;
    return -100;
}

std::shared_ptr<peg::Ast> parse_input(peg::parser peg, const std::string& codeText, const std::string& fileName, bool bench) {
    std::string grammarErrors;
    std::string codeErrors = fileName + "\n";
    std::string astOptimizedText;

    auto p1 = std::chrono::high_resolution_clock::now();
    std::shared_ptr<peg::Ast> ast;
    auto ret = parse_code(codeText, peg, codeErrors, ast);
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
    } else {
        std::cout << "Syntax errors: " << std::endl << codeErrors << std::endl;
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
                std::cout << "AST:" << std::endl << peg::ast_to_s(ast) << std::endl;
            }
        }

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        if (bench) {
            std::cout << fileName << (ast ? " PASS" : " FAIL") << " in " << duration.count() << "us" << std::endl << std::endl;
        } else {
            std::cout << fileName << (ast ? " PASS" : " FAIL") << std::endl;
        }

        rc = rc && ast;
        pos++;
    }

    return rc ? 0 : -1;
}
