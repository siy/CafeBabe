//
//  ast_opt.cpp
//
//  Copyright (c) 2021 Sergiy Yevtushenko. All rights reserved.
//  MIT License
//

#pragma once
#include <peglib.h>

// Parser API
peg::parser load_parser(bool bench);
std::shared_ptr<peg::Ast> parse_input(peg::parser peg, const std::string &codeText, const std::string &fileName, bool bench);

// AST optimizer static data
extern std::set<std::string> filter_safe;
extern std::set<std::string> filter_if_empty;
extern std::set<std::string> filter_unsafe;
extern std::set<std::string> unsafe_parents;

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
