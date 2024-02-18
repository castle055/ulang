//
// Created by castle on 2/10/24.
//

#ifndef ULANG_TOKENS_H
#define ULANG_TOKENS_H

#include "keywords.h"

#include <typeinfo>
#include <utility>
#include <optional>

namespace ulang::lexer {
  struct token_t {
    [[nodiscard]] const std::type_info& type() const { return typeid(*this); }
    
    template <typename TokenType>
    requires std::is_base_of_v<token_t, TokenType>
    [[nodiscard]] bool is_type() const {
      return type() == typeid(TokenType);
    }
    template <typename TokenType>
    requires std::is_base_of_v<token_t, TokenType>
    [[nodiscard]] std::optional<const TokenType*> as() const {
      if (is_type<TokenType>()) {
        return std::optional{(const TokenType*)this};
      } else {
        return std::nullopt;
      }
    }
    
    virtual std::string get_text() = 0;
  };

#define TOKEN_TEXT(TEXT) std::string get_text() override { return TEXT; }
  
  template<class N, class As = token_t, typename... Args>
  inline As* create_token(Args... args) {
    return new N(std::forward<Args>(args)...);
  }
  
  struct start_token_t: public token_t { TOKEN_TEXT("$") };
  struct end_token_t: public token_t { TOKEN_TEXT("$") };
  
  namespace symbol {
    struct left_parenthesis_t: public token_t { TOKEN_TEXT("(") };
    struct right_parenthesis_t: public token_t { TOKEN_TEXT(")") };
    struct left_curly_bracket_t: public token_t { TOKEN_TEXT("{") };
    struct right_curly_bracket_t: public token_t { TOKEN_TEXT("}") };
    struct left_square_bracket_t: public token_t { TOKEN_TEXT("[") };
    struct right_square_bracket_t: public token_t { TOKEN_TEXT("]") };
    struct left_angle_bracket_t: public token_t { TOKEN_TEXT("<") };
    struct right_angle_bracket_t: public token_t { TOKEN_TEXT(">") };
    
    struct plus_t: public token_t { TOKEN_TEXT("+") };
    struct minus_t: public token_t { TOKEN_TEXT("-") };
    struct star_t: public token_t { TOKEN_TEXT("*") };
    
    struct forward_slash_t: public token_t { TOKEN_TEXT("/") };
    struct backward_slash_t: public token_t { TOKEN_TEXT("\\") };
    
    struct equal_t: public token_t { TOKEN_TEXT("=") };
    
    struct negation_t: public token_t { TOKEN_TEXT("!") };
    
    struct ampersand_t: public token_t { TOKEN_TEXT("&") };
    struct pipe_t: public token_t { TOKEN_TEXT("|") };
    struct hat_t: public token_t { TOKEN_TEXT("^") };
    
    struct colon_t: public token_t { TOKEN_TEXT(":") };
    struct semicolon_t: public token_t { TOKEN_TEXT(";") };
    struct comma_t: public token_t { TOKEN_TEXT(",") };
    struct dot_t: public token_t { TOKEN_TEXT(".") };
  }
  
  template <keywords::keyword Keyword = keywords::ANY_KEYWORD>
  struct keyword_token_t: public token_t {
    keyword_token_t() {
      static_assert(!std::is_same_v<Keyword, keywords::ANY_KEYWORD>, "Cannot instantiate ANY_KEYWORD");
      if constexpr (!std::is_same_v<Keyword, keywords::ANY_KEYWORD>) {
        text = Keyword::text;
      }
    }
    
    std::string text;
    TOKEN_TEXT(text)
  };
  
  struct identifier_token_t: public token_t {
    explicit identifier_token_t(std::string  _text): text(std::move(_text)) {}
    
    std::string text;
    TOKEN_TEXT(text)
  };
  
  template <typename T>
  struct literal_t: public token_t {
    explicit literal_t(T _value): value(_value) {}
    
    T value;
    TOKEN_TEXT(std::to_string(value))
  };
  using floating_literal_t = literal_t<double>;
  using integer_literal_t = literal_t<long>;
  using string_literal_t = literal_t<std::string>;
  
}

#endif //ULANG_TOKENS_H
