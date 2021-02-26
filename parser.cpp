//
//  parser.cpp
//
//  Copyright (c) 2021 Sergiy Yevtushenko. All rights reserved.
//  MIT License
//

#include "cafebabe.h"

// CafeBabe grammar
extern std::string grammarText;

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

std::shared_ptr<peg::Ast> parse_input(peg::parser peg, const std::string &codeText, const std::string &fileName, bool bench) {
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
