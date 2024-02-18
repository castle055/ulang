//
// Created by castle on 2/10/24.
//

#ifndef ULANG_KEYWORDS_H
#define ULANG_KEYWORDS_H

#include <string>

namespace ulang::keywords {
  struct keyword_t {};
  
  template <typename T>
  concept keyword = std::derived_from<T, keyword_t>;
  
  struct ANY_KEYWORD: public keyword_t {
  };
  
  struct fn: public keyword_t {
    static constexpr std::string text = "fn";
  };
  struct let: public keyword_t {
    static constexpr std::string text = "let";
  };
  struct ret: public keyword_t {
    static constexpr std::string text = "ret";
  };
  struct if_: public keyword_t {
    static constexpr std::string text = "if";
  };
  struct else_: public keyword_t {
    static constexpr std::string text = "else";
  };
  struct for_: public keyword_t {
    static constexpr std::string text = "for";
  };
  struct while_: public keyword_t {
    static constexpr std::string text = "while";
  };
  struct when: public keyword_t {
    static constexpr std::string text = "when";
  };
  struct type: public keyword_t {
    static constexpr std::string text = "type";
  };
  struct trait: public keyword_t {
    static constexpr std::string text = "trait";
  };
  struct impl: public keyword_t {
    static constexpr std::string text = "impl";
  };
  struct as: public keyword_t {
    static constexpr std::string text = "as";
  };
}

#endif //ULANG_KEYWORDS_H
