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
#include <iostream>
#include <cstring>

#include <CafeBabe.h>

//class tree_holder {
//    Tree *ptr;
//public:
//    tree_holder(Tree* ptr_){
//        this->ptr = ptr_;
//    }
//    ~tree_holder() {
//        cnez_free(ptr);
//    }
//
//    Tree* tree() {
//        return ptr;
//    }
//};
//
//class AstNode {
//    std::string name;
//    std::string text;
//    std::vector<AstNode> children;
//    bool _is_failure;
//public:
//    AstNode(Tree* node):name(CafeBabe_tag(node->tag)), text(node->text, node->text + node->len), _is_failure(false) {
//        for(size_t i = 0; i < node->size; i++) {
//            children.emplace_back(node->childs[i]);
//        }
//    }
//
//    AstNode():_is_failure(true) {
//    }
//
//    template<typename T>
//    const AstNode& dump(T& out) {
//        dump(out, 0);
//        out << std::endl;
//        return *this;
//    }
//
//    bool isFailure() {
//        return _is_failure;
//    }
//
//private:
//    template<typename T> void dump(T& out, int depth) {
//        out << std::endl;
//
//        for(int i = 0; i < depth; i++) {
//            out << ' ';
//        }
//
//        out << "[#" << this->name;
//
//        if(this->children.empty()) {
//            out << " '" << this->text << "'";
//        } else {
//            for(auto child : children) {
//                out << " ";
//                child.dump(out, depth + 1);
//            }
//        }
//        out << "]";
//    }
//};

//AstNode parse(std::string input) {
//    auto ptr = (Tree*) CafeBabe_parse(input.c_str(), input.size(), nullptr, nullptr, nullptr, nullptr, nullptr);
//    tree_holder parseResult { ptr};
//    return parseResult.tree() ? AstNode(parseResult.tree()) : AstNode();
//}

input_reader_t* create_reader(std::string source) {
    auto ptr = new input_reader_t ;
    auto text = strdup(source.data());
    ptr->ptr = ptr->input = text;
    return ptr;
}

void destroy_reader(input_reader_t* reader) {
    free((void *) reader->input);
    free(reader);
}

int main(int argc, const char **argv) {
    bool verbose = false;
    bool bench = false;
    int i;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            break;
        }

        if (strlen(argv[i]) != 2) {
            std::cout << "Invalid option" << argv[i] << std::endl;
        }

        switch (argv[i][1]) {
            case 'v': verbose = true; break;
            case 'b': bench = true; break;
            default:
                std::cout << "Unknown option" << argv[i] << ", ignored" << std::endl;
        }
    }

    for(; i < argc; i++) {
        {
            std::ifstream t(argv[i]);

            if (!t.is_open()) {
                std::cout << "Unable to open input file " << argv[i] << " FAIL " << std::endl;
                continue;
            }

            std::string input((std::istreambuf_iterator<char>(t)),
                            std::istreambuf_iterator<char>());

            auto reader = create_reader(input);
            auto context = pcc_create(reader);
            int result = 0;
            auto rc = pcc_parse(context, &result);
            pcc_destroy(context);
            destroy_reader(reader);

            if (bench) {
                std::cout << "rc: " << rc << ", result: " << result << std::endl;
            }

//            if (verbose) {
//                result.dump(std::cout);
//            }

            std::cout << argv[i] << " - " << (rc ? "FAIL" : "PASS") << std::endl;
        }
    }
}
