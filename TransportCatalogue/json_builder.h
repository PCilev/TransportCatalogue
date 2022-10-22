#pragma once

#include <vector>
#include <string>
#include <variant>

#include "json.h"

namespace json {
    
class BaseContext;
class KeyItemContext;
class BeginItemContext;
class ArrayItemContext;
class DictItemContext;
    
using ValueContext = std::variant<BeginItemContext, ArrayItemContext, DictItemContext>;

class Builder {
public:
    Builder() = default;
    
    KeyItemContext Key(const std::string&);
    
    Builder& Value(const Node&);
    
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    
    Builder& EndDict();
    Builder& EndArray();
    
    Node Build();
    
    std::vector<Node*>& GetStack() {
        return nodes_stack_;
    }
    const std::vector<Node*>& GetStack() const {
        return nodes_stack_;
    }
private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    bool is_begin = false;
};
    

class DictItemContext {
public:
    DictItemContext(Builder& builder) : builder_{builder} {
    }
    
    KeyItemContext Key(const std::string& str);
    Builder& EndDict();
    
private:
    Builder& builder_;
};
    
//class ValueItemContext : public DictItemContext{};
    
class KeyItemContext {
public:
    KeyItemContext(Builder& builder) : builder_{builder} {
    }
    
    DictItemContext Value(const Node& node);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    
private:
    Builder& builder_;
};
    
class ArrayItemContext {
public:
    ArrayItemContext(Builder& builder) : builder_{builder} {
    }
    
    ArrayItemContext& Value(const Node& node);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
    
private:
    Builder& builder_;
};
    
} // namespace json