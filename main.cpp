//
//  main.cpp
//
//  Copyright (c) 2020 Sergiy Yevtushenko. All rights reserved.
//  MIT License
//

#include <functional>
#include <fstream>
#include <sstream>
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
    peg.log = makeErrorLogger(errorText);
    return peg.parse_n(text.data(), text.size(), ast);
}

struct lint_result {
    std::string grammarErrors;
    std::string codeErrors;
    std::string astOptimized;
};

bool lint(const std::string &grammarText, const std::string &codeText, std::string &fileName, lint_result& result, bool opt_mode = true) {
    std::string grammarErrors;
    std::string codeErrors = fileName + "\n";
    std::string astOptimizedText;

    peg::parser peg;
    auto ret = parse_grammar(grammarText, peg, grammarErrors);

    if (ret && peg) {
        std::shared_ptr<peg::Ast> ast;
        ret = parse_code(codeText, peg, codeErrors, ast);
        if (ast) {
            astOptimizedText = peg::ast_to_s(peg.optimize_ast(ast, opt_mode));
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
    bool verbose = false;

    if (argv[pos][0] == '-' && argv[pos][1] == 'v') {
        verbose = true;
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

    auto rc = lint(grammar, code, fileName, result, true);

    if (verbose) {
        if (rc) {
            std::cout << "AST:"  << std::endl << result.astOptimized << std::endl;
        } else {
            std::cout << "Errors: " << std::endl << result.codeErrors << std::endl;
        }
    }

    std::cout << fileName << (rc ? " PASS" : " FAIL") << std::endl;

    return rc ? 0 : -1;
}
