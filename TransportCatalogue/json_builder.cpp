#include "json_builder.h"

#include <utility>
#include <stdexcept>
#include <iostream>

using namespace std;

namespace json {
    
KeyItemContext Builder::Key(const std::string& str) {
    if (is_begin || !(nodes_stack_.size() && nodes_stack_.back()->IsDict())) {
        throw logic_error("error Key method ");
    }
    nodes_stack_.push_back(new Node(str));
    return KeyItemContext(*this);
}

Builder& Builder::Value(const Node& node) {
    if (is_begin) {
        throw logic_error("error Value method ");
    }
    
    if (nodes_stack_.empty()) {
        root_ = node;
        is_begin = true;
        return *this;
    }
    else if (nodes_stack_.back()->IsArray()) {
        nodes_stack_.back()->AsArray().push_back(node);
        return *this;
    }
    else {
        nodes_stack_[nodes_stack_.size()-2]->AsDict()[move(nodes_stack_.back()->AsString())] = node;
        nodes_stack_.pop_back();
        return *this;
    }
}
    
DictItemContext Builder::StartDict() {
    if (is_begin || (!nodes_stack_.empty() && (!nodes_stack_.back()->IsArray() &&
        !nodes_stack_.back()->IsString()))) {
        throw logic_error("error StartDict method ");
    }
    nodes_stack_.push_back(new Node(Dict()));
    return DictItemContext(*this);
}
    
ArrayItemContext Builder::StartArray() {
    if (is_begin || (!nodes_stack_.empty() && (!nodes_stack_.back()->IsArray() &&
        !nodes_stack_.back()->IsString()))) {
        throw logic_error("error StartArray method ");
    }
    nodes_stack_.push_back(new Node(Array()));
    return ArrayItemContext(*this);
}
    
Builder& Builder::EndDict() {
    if (is_begin || nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        throw logic_error("error EndDict method ");
    }
    Node node = move(*(nodes_stack_.back()));
    nodes_stack_.pop_back();
    return Value(node);
}
    
Builder& Builder::EndArray() {
    if (is_begin || nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw logic_error("error EndArray method ");
    }
    Node node = move(*(nodes_stack_.back()));
    nodes_stack_.pop_back();
    return Value(node);
}
    
Node Builder::Build() {
    if (!is_begin) {
        throw logic_error("error EndDict method ");
    }
    return move(root_);
}
    
    
KeyItemContext DictItemContext::Key(const std::string& str) {
        return builder_.Key(str);
}
    
Builder& DictItemContext::EndDict() {
        return builder_.EndDict();
}
    
    
DictItemContext KeyItemContext::Value(const Node& node) {
    std::vector<Node*>& stack = builder_.GetStack();
    stack[stack.size()-2]->AsDict()[move(stack.back()->AsString())] = node;
    stack.pop_back();
    return DictItemContext(builder_);
}
    
DictItemContext KeyItemContext::StartDict() {
    return builder_.StartDict();
}
    
ArrayItemContext KeyItemContext::StartArray() {
    return builder_.StartArray();
}
    
    
ArrayItemContext& ArrayItemContext::Value(const Node& node) {
    builder_.GetStack().back()->AsArray().push_back(node);
    return *this;
}
    
DictItemContext ArrayItemContext::StartDict() {
    return builder_.StartDict();
}
    
ArrayItemContext ArrayItemContext::StartArray() {
    return builder_.StartArray();
}
    
Builder& ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

} //namespace json