//
//  symbol_table.h
//
//  Copyright (c) 2021 Sergiy Yevtushenko. All rights reserved.
//  MIT License
//

#pragma once

#include <string>
#include <set>
#include <map>
#include <memory>
#include <iostream>
#include <cstring>

using namespace std;

enum class symbol_type {
    TYPE,
    CONST,
    VAR,
    TYPE_VAR
};

std::ostream &operator<<(std::ostream &os, symbol_type const &m) {
    static const char* TypeNames[] = {"Type", "Const", "Var", "TypeVar"};

    return os << TypeNames[static_cast<int>(m)];
}

struct symbol {
    string name;
    symbol_type type;
    size_t line;
    size_t column;
};

std::ostream &operator<<(std::ostream &os, symbol const &m) {
    return os << "[" << m.name << "] " << m.type << " (defined at " << m.line << ":" << m.column << ")" << endl;
}

bool operator==(const struct symbol &l, const struct symbol &r) {
    return l.name == r.name && l.type == r.type;
}

class symbol_table_node {
    struct symbol symbol;
    symbol_table_node *next;

public:
    symbol_table_node() {
        next = nullptr;
    }

    symbol_table_node(struct symbol symbol) {
        this->symbol = move(symbol);
        this->next = nullptr;
    }

    friend class symbol_table;
};

class symbol_table {
    static constexpr size_t NUM_ROOTS = 53;

    symbol_table_node *head[NUM_ROOTS];
    symbol_table* prev;

    static constexpr hash<string> hasher {};

    static size_t calc_index(const string& id) {
        return hasher(id) % NUM_ROOTS;
    }

public:
    symbol_table(symbol_table* prev) {
        this->prev = prev;
        memset(head, 0, sizeof(head));
    }

    bool insert_global(const struct symbol& symbol) {
        auto prev_table = this;

        while (prev_table->prev != nullptr) {
            prev_table = prev_table->prev;
        }

        return prev_table->insert(symbol);
    }

    bool insert(const struct symbol& symbol) {
        int index = calc_index(symbol.name);
        auto p = head[index];

        // make sure there is no other such symbol
        while (p != nullptr) {
            if (p->symbol == symbol) {
                return false;
            }

            p = p->next;
        }

        auto node = new symbol_table_node(symbol);
        node -> next = head[index];
        head[index] = node;

        return true;
    }

    optional<symbol> find(const string& id) {
        auto p = head[calc_index(id)];

        while (p != nullptr) {
            if (p->symbol.name == id) {
                return p->symbol;
            }

            p = p->next;
        }

        return (prev) ? prev->find(id) : nullopt;
    }

    void print(std::ostream &os) {
        for (size_t i = 0; i < NUM_ROOTS; i++) {
            auto p = head[i];

            while (p != nullptr) {
                os << " " << p->symbol << endl;
                p = p->next;
            }
        }
    }
};
