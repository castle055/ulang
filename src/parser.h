//
// Created by castle on 2/10/24.
//

#ifndef ULANG_PARSER_H
#define ULANG_PARSER_H

#include "ast.h"
#include "lexer.h"

#include "streams.h"

namespace ulang {
  class parser_t {
  public:
    explicit parser_t(char_stream_t& input_stream)
    : token_stream(input_stream) { }
    
    std::unique_ptr<ast::node::base_t> operator()() {
      std::unique_ptr<ast::node::base_t> ast = parse_module().value();
      return ast;
    }
    
  private:
    lexer::token_stream_t token_stream;
  private:
    template<class TokenType>
    static bool is_token_type(const lexer::token_t& token) {
      return token.is_type<TokenType>();
    }
    template<class TokenType>
    bool is_current_token_type() {
      return is_token_type<TokenType>(token_stream.current());
    }
  
  public:
    struct parser_error_t {
    };
    inline std::unexpected<parser_error_t> parser_error(parser_error_t error) {
      return std::unexpected(error);
    }
    
    template<typename R>
    using parser_result_t = std::expected<R, parser_error_t>;
    
  private: /* * PARSING FUNCTIONS * */
    
    parser_result_t<std::unique_ptr<ast::node::module_t>> parse_module() {
      std::vector<ast::node::module_t::allowed_nodes> members{};
      
      while (true/* token_stream.end() */) {
        lexer::token_t& current = token_stream.current();
        if (is_token_type<lexer::start_token_t>(current)) {
          token_stream.advance();
        } else if (is_token_type<lexer::keyword_token_t<keywords::fn>>(current)) {
          auto res = parse_fn_declare();
          if (res) {
            members.emplace_back(std::move(res.value()));
          } else return parser_error (res.error());
        } else if (is_token_type<lexer::keyword_token_t<keywords::type>>(current)) {
          auto res = parse_type_declare();
          if (res) {
            members.emplace_back(std::move(res.value()));
          } else return parser_error (res.error());
        } else if (is_token_type<lexer::keyword_token_t<keywords::trait>>(current)) {
          auto res = parse_trait_declare();
          if (res) {
            members.emplace_back(std::move(res.value()));
          } else return parser_error (res.error());
        } else if (is_token_type<lexer::keyword_token_t<keywords::impl>>(current)) {
          auto res = parse_trait_impl();
          if (res) {
            members.emplace_back(std::move(res.value()));
          } else return parser_error (res.error());
        } else if (is_token_type<lexer::keyword_token_t<keywords::let>>(current)) {
          auto res = parse_let_declare();
          if (res) {
            members.emplace_back(std::move(res.value()));
          } else return parser_error (res.error());
        }
      }
      
      return std::make_unique<ast::node::module_t>(members);
    }
    
    parser_result_t<std::unique_ptr<ast::node::fn_declare_t>> parse_fn_declare() {
      auto m = token_stream.mark_current();
      token_stream.advance();
      
      if (!is_current_token_type<lexer::identifier_token_t>()) {
        token_stream.goto_marker(m);
        return parser_error ({});
      }
      
      auto id = token_stream.current().as<lexer::identifier_token_t>().value()->text;
      token_stream.advance();
      
      if (is_current_token_type<lexer::symbol::left_angle_bracket_t>()) {
        // * Parse Generics
      }
      
      std::vector<std::unique_ptr<ast::node::fn_declare_param_t>> params{};
      if (is_current_token_type<lexer::symbol::left_parenthesis_t>()) {
        // * Parse Params
        auto res = parse_fn_declare_params();
        if (res) {
          params = std::move(res.value());
        } else return parser_error (res.error());
      }
      
      std::optional<std::unique_ptr<ast::node::type_expr_t>> return_type = std::nullopt;
      if (is_current_token_type<lexer::symbol::colon_t>()) {
        // * Parse Return Type
        token_stream.advance();
        auto res = parse_type_expr();
        if (res) {
          return_type = std::move(res.value());
        } else return parser_error (res.error());
      }
      
      std::unique_ptr<ast::node::expr_sequence_t> body;
      // * Parse Body
      
      return std::make_unique<ast::node::fn_declare_t>(id,
                                                       return_type,
                                                       params,
                                                       body);
    }
    parser_result_t<std::vector<std::unique_ptr<ast::node::fn_declare_param_t>>> parse_fn_declare_params() {
      auto m = token_stream.mark_current();
      std::vector<std::unique_ptr<ast::node::fn_declare_param_t>> params{};
      
      return params;
    }
    
    parser_result_t<std::unique_ptr<ast::node::type_declare_t>> parse_type_declare() {
      auto m = token_stream.mark_current();
      token_stream.advance();
      
      if (!is_current_token_type<lexer::identifier_token_t>()) {
        token_stream.goto_marker(m);
        return parser_error ({});
      }
      
      auto id = token_stream.current().as<lexer::identifier_token_t>().value()->text;
      token_stream.advance();
      
      std::unique_ptr<ast::node::type_expr_t>  expr;
      if (is_current_token_type<lexer::symbol::left_curly_bracket_t>()) {
        // * Parse Struct Type
      } else if (is_current_token_type<lexer::symbol::equal_t>()) {
        // * Parse Arithmetic Type Expression
        auto res = parse_type_expr();
        if (res) {
          expr = std::move(res.value());
        } else return parser_error (res.error());
      } else {
        token_stream.goto_marker(m);
        return parser_error ({});
      }
      
      return std::make_unique<ast::node::type_declare_t>(id,
                                                         expr);
    }
    
    parser_result_t<std::unique_ptr<ast::node::trait_declare_t>> parse_trait_declare() {
      auto m = token_stream.mark_current();
      token_stream.advance();
      
      if (!is_current_token_type<lexer::identifier_token_t>()) {
        token_stream.goto_marker(m);
        return parser_error ({});
      }
      
      auto id = token_stream.current().as<lexer::identifier_token_t>().value()->text;
      token_stream.advance();
      
      std::vector<std::unique_ptr<ast::node::fn_declare_t>> functions;
      // * Parse Trait Functions
      
      return std::make_unique<ast::node::trait_declare_t>(id,
                                                          functions);
    }
    
    parser_result_t<std::unique_ptr<ast::node::trait_impl_t>> parse_trait_impl() {
      auto m = token_stream.mark_current();
      token_stream.advance();
      
      if (!is_current_token_type<lexer::identifier_token_t>()) {
        token_stream.goto_marker(m);
        return parser_error ({});
      }
      
      auto type_id = token_stream.current().as<lexer::identifier_token_t>().value()->text;
      token_stream.advance();
      
      
      std::optional<std::string> trait_id = std::nullopt;
      if (is_current_token_type<lexer::keyword_token_t<keywords::as>>()) {
        token_stream.advance();
        
        if (!is_current_token_type<lexer::identifier_token_t>()) {
          token_stream.goto_marker(m);
          return parser_error ({});
        }
        
        trait_id = token_stream.current().as<lexer::identifier_token_t>().value()->text;
        token_stream.advance();
      }
      
      std::vector<std::unique_ptr<ast::node::fn_declare_t>> functions;
      // * Parse Trait Functions
      
      return std::make_unique<ast::node::trait_impl_t>(type_id,
                                                       trait_id,
                                                       functions);
    }
    
    parser_result_t<std::unique_ptr<ast::node::let_declare_t>> parse_let_declare() {
      auto m = token_stream.mark_current();
      token_stream.advance();
      
      if (!is_current_token_type<lexer::identifier_token_t>()) {
        token_stream.goto_marker(m);
        return parser_error ({});
      }
      
      auto id = token_stream.current().as<lexer::identifier_token_t>().value()->text;
      token_stream.advance();
      
      std::optional<std::unique_ptr<ast::node::type_expr_t>> value_type = std::nullopt;
      if (is_current_token_type<lexer::symbol::colon_t>()) {
        // * Parse Return Type
      }
      
      std::optional<std::unique_ptr<ast::node::expr_t>> assignment = std::nullopt;
      // * Parse Assignment
      
      return std::make_unique<ast::node::let_declare_t>(id,
                                                        value_type,
                                                        assignment);
    }
    
    parser_result_t<std::unique_ptr<ast::node::expr_t>> parse_expr() {
      return std::make_unique<ast::node::expr_t>();
    }
    
    parser_result_t<std::unique_ptr<ast::node::type_expr_t>> parse_type_expr() {
      return std::make_unique<ast::node::type_expr_t>();
    }
  };
}

#endif //ULANG_PARSER_H
