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
    peg.enable_ast();
    peg.enable_packrat_parsing();
    peg.log = makeErrorLogger(errorText);
    return peg.parse_n(text.data(), text.size(), ast);
}

struct lint_result {
    std::string grammarErrors;
    std::string codeErrors;
    std::string astOptimized;
};

static std::set<std::string> filter_safe{
        "__",
        "BlockEnd",
        "BlockStart",
        "LP",
        "RP",
        "Semicolon",
        "SequenceSign"
};

static std::set<std::string> filter_if_empty{"OptionalEllipsis"};

static std::set<std::string> filter_unsafe{
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

static std::set<std::string> unsafe_parents{
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

bool lint(const std::string &grammarText, 
          const std::string &codeText, 
          std::string &fileName, 
          lint_result &result, 
          bool verbose) {
    std::string grammarErrors;
    std::string codeErrors = fileName + "\n";
    std::string astOptimizedText;

    peg::parser peg;

    auto p0 = std::chrono::high_resolution_clock::now();
    auto ret = parse_grammar(grammarText, peg, grammarErrors);
    auto p1 = std::chrono::high_resolution_clock::now();

    if (verbose) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(p1 - p0);
        std::cout << "parse grammar: " << duration.count() << "us" << std::endl;
    }

    if (ret && peg) {
        std::shared_ptr<peg::Ast> ast;
        ret = parse_code(codeText, peg, codeErrors, ast);
        auto p2 = std::chrono::high_resolution_clock::now();

        if (verbose) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(p2 - p1);
            std::cout << "parse code: " << duration.count() << "us" << std::endl;
        }

        if (ast) {
            auto optimized = optimize(true, peg.optimize_ast(ast));
            auto p3 = std::chrono::high_resolution_clock::now();

            if (verbose) {
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(p3 - p2);
                std::cout << "optimize AST: " << duration.count() << "us" << std::endl;
            }

            astOptimizedText = peg::ast_to_s(optimized);
        }
    }

    result.grammarErrors = grammarErrors;

    if (!codeErrors.empty()) {
        result.codeErrors = codeErrors;
        result.astOptimized = astOptimizedText;
    }

    return ret;
}

extern std::string grammarText;

int main(int argc, const char **argv) {
    if (argc <= 1) {
        std::cout << "Usage: cbc \"text\" " << std::endl;
        return -100;
    }

    int pos = 1;
    bool verbose = true;

    if (argv[pos][0] == '-' && argv[pos][1] == 'q') {
        verbose = false;
        pos++;
    }

    std::string grammar(grammarText);
    std::string fileName = argv[pos];
    std::ifstream ifs2(fileName);

    if (!ifs2.is_open()) {
        std::cout << fileName << " MISSING" << std::endl;
        return -2;
    }

    std::string code((std::istreambuf_iterator<char>(ifs2)),
                     (std::istreambuf_iterator<char>()));

    lint_result result;

    if (verbose) {
        std::cout << "Code:" << std::endl << code << std::endl;
    }

    auto start = std::chrono::high_resolution_clock::now();
    auto rc = lint(grammar, code, fileName, result, verbose);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    if (verbose) {
        if (rc) {
            std::cout << "AST:" << std::endl << result.astOptimized << std::endl;
        } else {
            std::cout << "Errors: " << std::endl << result.codeErrors << std::endl;
        }
    }

    std::cout << fileName << (rc ? " PASS" : " FAIL") << " in " << duration.count() << "ms" << std::endl;

    return rc ? 0 : -1;
}
