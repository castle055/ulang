//
// Created by castle on 2/9/24.
//

#ifndef ULANG_AST_H
#define ULANG_AST_H

#include <iostream>

#include <type_traits>
#include <typeinfo>
#include <memory>
#include <string>
#include <utility>
#include <optional>
#include <variant>
#include <vector>


namespace ulang::ast {
  namespace node {
    enum class type_e {
      EXPR_LITERAL,
      BINARY_EXPR,
      LET_DECL,
      EXPR_SEQUENCE,
      IF_ELSE,
      FOR_SPEC,
      FOR,
      WHILE,
      WHEN_PATTERN,
      WHEN,
      RETURN,
      FN_DECL_PARAM,
      FN_DECL,
      TYPE_DECL,
      TRAIT_DECL,
      TRAIT_IMPL,
      MODULE,
    };
    
    struct base_t {
      const type_e type;
      explicit base_t(type_e _type): type(_type) {}
      
      virtual ~base_t() = default;
    };
    
    //template<class N, class As = N, typename... Args>
    //inline std::unique_ptr<As> create(Args... args) {
    //  return std::make_unique<N>(std::forward<Args>(args)...);
    //}
    
    struct expr_t: public base_t {
      explicit expr_t(type_e _type): base_t(_type) {}
    };
    
    struct type_expr_t: public base_t {
      explicit type_expr_t(type_e _type): base_t(_type) {}
    };
    
    struct binary_expr_t: public expr_t {
      std::string op;
      std::unique_ptr<expr_t> lhs, rhs;
      binary_expr_t(std::string _op, std::unique_ptr<expr_t> _lhs,std::unique_ptr<expr_t> _rhs)
      : expr_t(type_e::BINARY_EXPR),
        op(std::move(_op)), lhs(std::move(_lhs)), rhs(std::move(_rhs)) {}
    };
    
    struct literal_t: public expr_t {
      long value;
      explicit literal_t(long _value)
      : expr_t(type_e::EXPR_LITERAL),
        value(_value) {}
    };
    
    struct let_declare_t: public expr_t {
      std::string id;
      std::optional<std::unique_ptr<type_expr_t>> value_type;
      std::optional<std::unique_ptr<expr_t>> assignment;
      explicit let_declare_t(std::string _id,
                             std::optional<std::unique_ptr<type_expr_t>>& _value_type,
                             std::optional<std::unique_ptr<expr_t>>& _assignment)
      : expr_t(type_e::LET_DECL),
        id(std::move(_id)),
        value_type(std::move(_value_type)),
        assignment(std::move(_assignment)) {}
    };
    
    struct expr_sequence_t: public base_t {
      std::vector<std::unique_ptr<expr_t>> exprs;
      explicit expr_sequence_t(std::vector<std::unique_ptr<expr_t>> _exprs)
      : base_t(type_e::EXPR_SEQUENCE),
        exprs(std::move(_exprs)) {}
    };
    
    // ? Control Flow
    struct if_else_t: public expr_t {
      std::unique_ptr<expr_t> condition;
      std::unique_ptr<expr_sequence_t> then;
      std::unique_ptr<expr_sequence_t> else_;
      explicit if_else_t(std::unique_ptr<expr_t>& _condition,
                         std::unique_ptr<expr_sequence_t>& _then,
                         std::unique_ptr<expr_sequence_t>& _else)
      : expr_t(type_e::IF_ELSE),
        condition(std::move(_condition)),
        then(std::move(_then)),
        else_(std::move(_else)) {}
    };
    
    struct for_spec_t: public base_t {
      std::unique_ptr<expr_t> init;
      std::unique_ptr<expr_t> condition;
      std::unique_ptr<expr_t> incrementer;
      explicit for_spec_t(std::unique_ptr<expr_t> _init,
                          std::unique_ptr<expr_t> _condition,
                          std::unique_ptr<expr_t> _incrementer)
      : base_t(type_e::FOR_SPEC),
        init(std::move(_init)),
        condition(std::move(_condition)),
        incrementer(std::move(_incrementer)) {}
    };
    
    struct for__t: public expr_t {
      std::unique_ptr<for_spec_t> spec;
      std::unique_ptr<expr_sequence_t> body;
      explicit for__t(std::unique_ptr<for_spec_t> _spec,
                     std::unique_ptr<expr_sequence_t> _body)
      : expr_t(type_e::FOR),
        spec(std::move(_spec)),
        body(std::move(_body)) {}
    };
    
    struct while__t: public expr_t {
      std::unique_ptr<expr_t> condition;
      std::unique_ptr<expr_sequence_t> body;
      explicit while__t(std::unique_ptr<expr_t> _condition,
                       std::unique_ptr<expr_sequence_t> _body)
      : expr_t(type_e::WHILE),
        condition(std::move(_condition)),
        body(std::move(_body)) {}
    };
    
    struct when_pattern_t: public base_t {
      std::unique_ptr<expr_t> pattern;
      std::unique_ptr<expr_sequence_t> body;
      explicit when_pattern_t(std::unique_ptr<expr_t> _pattern,
                              std::unique_ptr<expr_sequence_t> _body)
      : base_t(type_e::WHEN_PATTERN),
        pattern(std::move(_pattern)),
        body(std::move(_body)) {}
    };
    
    struct when_t: public expr_t {
      std::unique_ptr<expr_t> target;
      std::vector<std::unique_ptr<when_pattern_t>> patterns;
      explicit when_t(std::unique_ptr<expr_t> _target,
                      std::vector<std::unique_ptr<when_pattern_t>> _patterns)
      : expr_t(type_e::WHEN),
        target(std::move(_target)),
        patterns(std::move(_patterns)) {}
    };
    
    struct return__t: public expr_t {
      std::unique_ptr<expr_t> expr;
      explicit return__t(std::unique_ptr<expr_t> _expr)
      : expr_t(type_e::RETURN),
        expr(std::move(_expr)) {}
    };
    
    // ? Functions
    struct fn_declare_param_t: public base_t {
      std::string id;
      std::unique_ptr<type_expr_t> value_type;
      std::optional<std::unique_ptr<expr_t>> default_value;
      explicit fn_declare_param_t(std::string _id,
                                  std::unique_ptr<type_expr_t> _value_type,
                                  std::unique_ptr<expr_t> _default_value)
      : base_t(type_e::FN_DECL_PARAM),
        value_type(std::move(_value_type)),
        default_value(_default_value? std::optional{std::move(_default_value)}: std::nullopt) {}
    };
    struct fn_declare_t: public expr_t {
      std::string id;
      std::optional<std::unique_ptr<type_expr_t>> return_type;
      std::vector<std::unique_ptr<fn_declare_param_t>> params;
      std::unique_ptr<expr_sequence_t> body;
      explicit fn_declare_t(std::string _id,
                            std::optional<std::unique_ptr<type_expr_t>>& _return_type,
                            std::vector<std::unique_ptr<fn_declare_param_t>>& _params,
                            std::unique_ptr<expr_sequence_t>& _body)
      : expr_t(type_e::FN_DECL),
        id(std::move(_id)),
        return_type(std::move(_return_type)),
        params(std::move(_params)),
        body(std::move(_body)) {}
    };
    
    // ? Type & Traits
    struct type_declare_t: public expr_t {
      std::string id;
      std::unique_ptr<type_expr_t> expr;
      explicit type_declare_t(std::string _id,
                              std::unique_ptr<type_expr_t>& _expr)
        : expr_t(type_e::TYPE_DECL),
          id(std::move(_id)),
          expr(std::move(_expr)) {}
    };
    
    struct trait_declare_t: public expr_t {
      std::string id;
      std::vector<std::unique_ptr<fn_declare_t>> functions;
      explicit trait_declare_t(std::string _id,
                              std::vector<std::unique_ptr<fn_declare_t>>& _functions)
        : expr_t(type_e::TRAIT_DECL),
          id(std::move(_id)),
          functions(std::move(_functions)) {}
    };
    
    struct trait_impl_t: public expr_t {
      std::string type_id;
      std::optional<std::string> trait_id;
      std::vector<std::unique_ptr<fn_declare_t>> functions;
      explicit trait_impl_t(std::string _type_id,
                            std::optional<std::string> _trait_id,
                            std::vector<std::unique_ptr<fn_declare_t>>& _functions)
        : expr_t(type_e::TRAIT_IMPL),
          type_id(std::move(_type_id)),
          trait_id(std::move(_trait_id)),
          functions(std::move(_functions)) {}
    };
    
    // ? Module
    struct module_t: public base_t {
      using allowed_nodes = std::variant<
        std::unique_ptr<fn_declare_t>,
        std::unique_ptr<type_declare_t>,
        std::unique_ptr<trait_declare_t>,
        std::unique_ptr<trait_impl_t>,
        std::unique_ptr<let_declare_t>
      >;
      
      std::vector<allowed_nodes> members;
      
      explicit module_t(std::vector<allowed_nodes>& _members)
        : base_t(type_e::MODULE),
          members(std::move(_members)) {}
    };
    
  }

#define VISITOR_DISPATCH_NODE(TYPE, NAME) \
  case node::type_e::TYPE:                \
    if (auto* downcast = dynamic_cast<node::NAME##_t*>(node)) \
      return visit_##NAME(downcast);      \
    else {std::cerr << "[ERROR] Couldn't downcast 'node::base_t*' to 'node::" #NAME "_t*: " << typeid(node).name() << std::endl; } \
    break
      
      
/*#define VISITOR_DISPATCH_NODE(TYPE, NAME) \
    case node::type_e::TYPE: return visit_##NAME((node::NAME##_t*)(node))
*/

#define VISITOR_METHOD_SIGNATURE(NAME) virtual Ret visit_##NAME(node::NAME##_t* NAME)
  
  template <typename Ret>
  struct visitor_base_t {
    explicit visitor_base_t(const std::unique_ptr<node::base_t>& _tree)
    : tree(_tree) {}
    
    virtual Ret operator()() {
      if (tree) { return visit(tree.get()); } else return {};
    }
  
  protected:
    Ret visit(node::base_t* node) {
      switch (node->type) {
        VISITOR_DISPATCH_NODE(EXPR_LITERAL, literal);
        VISITOR_DISPATCH_NODE(BINARY_EXPR, binary_expr);
        VISITOR_DISPATCH_NODE(LET_DECL, let_declare);
        VISITOR_DISPATCH_NODE(EXPR_SEQUENCE, expr_sequence);
        VISITOR_DISPATCH_NODE(IF_ELSE, if_else);
        VISITOR_DISPATCH_NODE(FOR_SPEC, for_spec);
        VISITOR_DISPATCH_NODE(FOR, for_);
        VISITOR_DISPATCH_NODE(WHILE, while_);
        VISITOR_DISPATCH_NODE(WHEN_PATTERN, when_pattern);
        VISITOR_DISPATCH_NODE(WHEN, when);
        VISITOR_DISPATCH_NODE(RETURN, return_);
        VISITOR_DISPATCH_NODE(FN_DECL_PARAM, fn_declare_param);
        VISITOR_DISPATCH_NODE(FN_DECL, fn_declare);
        VISITOR_DISPATCH_NODE(TYPE_DECL, type_declare);
        VISITOR_DISPATCH_NODE(TRAIT_DECL, trait_declare);
        VISITOR_DISPATCH_NODE(TRAIT_IMPL, trait_impl);
        VISITOR_DISPATCH_NODE(MODULE, module);
      }
      return {};
    }
    template <class T>
    Ret visit(std::unique_ptr<T>& node) {
      return visit(node.get());
    }
    
  protected:
    VISITOR_METHOD_SIGNATURE(binary_expr) { return {}; }
    VISITOR_METHOD_SIGNATURE(literal)     { return {}; }
    
    VISITOR_METHOD_SIGNATURE(let_declare) { return {}; }
    
    VISITOR_METHOD_SIGNATURE(expr_sequence) { return {}; }
    VISITOR_METHOD_SIGNATURE(if_else) { return {}; }
    VISITOR_METHOD_SIGNATURE(for_spec) { return {}; }
    VISITOR_METHOD_SIGNATURE(for_) { return {}; }
    VISITOR_METHOD_SIGNATURE(while_) { return {}; }
    VISITOR_METHOD_SIGNATURE(when_pattern) { return {}; }
    VISITOR_METHOD_SIGNATURE(when) { return {}; }
    VISITOR_METHOD_SIGNATURE(return_) { return {}; }
    VISITOR_METHOD_SIGNATURE(fn_declare_param) { return {}; }
    VISITOR_METHOD_SIGNATURE(fn_declare) { return {}; }
    VISITOR_METHOD_SIGNATURE(type_declare) { return {}; }
    VISITOR_METHOD_SIGNATURE(trait_declare) { return {}; }
    VISITOR_METHOD_SIGNATURE(trait_impl) { return {}; }
    VISITOR_METHOD_SIGNATURE(module) { return {}; }
  private:
    const std::unique_ptr<node::base_t>& tree;
  };

#undef VISITOR_DISPATCH_NODE
#undef VISITOR_METHOD_SIGNATURE
}

struct print_ast: public ulang::ast::visitor_base_t<std::vector<std::string>> {
  explicit print_ast(const std::unique_ptr<ulang::ast::node::base_t>& _tree)
    : visitor_base_t(_tree) {}
  
protected:
  std::vector<std::string> visit_module(ulang::ast::node::module_t *module) override {
    return {};
  }
};

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

using namespace ulang::ast;
using namespace llvm;
struct codegen_visitor_t: public visitor_base_t<llvm::Value*> {
  std::unique_ptr<llvm::LLVMContext> ctx = std::make_unique<llvm::LLVMContext>();
  std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("My Module", *ctx);
  std::unique_ptr<llvm::IRBuilder<>> builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
  
  std::unordered_map<std::string, Value*> symbols{};
  
  explicit codegen_visitor_t(const std::unique_ptr<node::base_t>& _tree)
  : visitor_base_t(_tree) {}

  void print() const {
    module->print(llvm::outs(), nullptr);
  }
protected:
  
  Value * visit_let_declare(node::let_declare_t *let_declare) override {
    if (let_declare->assignment) {
      auto assignment = visit(let_declare->assignment.value());
      
    }
    auto a = ConstantData::getIntegerValue(Type::getInt32Ty(*ctx), APInt(32, 2));
    auto b = ConstantData::getIntegerValue(Type::getInt32Ty(*ctx), APInt(32, 25));
    auto r = builder->CreateMul(a, b);
    //BasicBlock *BB = BasicBlock::Create(*ctx, "entry");
    //builder->SetInsertPoint(BB);
    //auto alloca = builder->CreateAlloca(Type::getInt32Ty(*ctx), nullptr, "a");
    //return builder->CreateLoad(alloca->getAllocatedType(), alloca, "a");
    return r;
  }
};

#endif //ULANG_AST_H
