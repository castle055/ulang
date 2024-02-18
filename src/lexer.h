//
// Created by castle on 2/10/24.
//

#ifndef ULANG_LEXER_H
#define ULANG_LEXER_H

#include "tokens.h"
#include "streams.h"
#include "char_utils.h"

#include "pretty_printer.h"

#include <deque>
#include <expected>

namespace ulang::lexer {
  struct lexer_error_t { };
  inline std::unexpected<lexer_error_t> lexer_error(lexer_error_t error) {
    return std::unexpected(error);
  }
  
  template <typename R>
  using lexer_result_t = std::expected<R, lexer_error_t>;
  
  struct token_stream_t: public stream_t<token_t, std::size_t> {
    explicit token_stream_t(char_stream_t& _input_stream)
    : input_stream(_input_stream) {
      
    }
    
    token_t & current() override {
      return **token_list_iterator;
    }
    token_t & advance() override {
      if (current_index < token_list.size()) {
        token_list_iterator++;
        current_index++;
        return current();
      } else if (current_index == token_list.size()) {
        if (input_stream.at_end()) {
          auto* t = create_token<end_token_t>();
          token_list.push_back(t);
          token_list_iterator++;
          return *t;
        }
        return *parse_token().transform([&] (token_t* t) {
          token_list.push_back(t);
          token_list_iterator++;
          current_index++;
          
          print_token(t);
          
          return t;
        }).value_or(&current());
      } else {
        // * This branch does not make any sense
        return current();
      }
    }
    token_t & backtrack() override {
      if (token_list_iterator != token_list.begin()) {
        token_list_iterator--;
        current_index--;
      }
      return current();
    }
    
    bool at_end() override {
      return current_index == token_list.size() && input_stream.at_end();
    }
    
    std::size_t mark_current() override {
      return current_index;
    }
    void goto_marker(const std::size_t &marker) override {
      while (marker < current_index) {
        backtrack();
      }
      while (marker > current_index) {
        advance();
      }
    }
  
  private:
    char_stream_t & input_stream;
    
    std::deque<token_t*> token_list{create_token<start_token_t>()};
    decltype(token_list)::iterator token_list_iterator = token_list.begin();
    std::size_t current_index = 0UL;
  
    pretty::multiline_printer_t<2> mp{};
    
    inline void print_token(token_t* t) {
      std::string s;
      if (t->is_type<start_token_t>()) {
        s = "$";
      } else if (t->is_type<identifier_token_t>()) {
        s = "\\ID/";
        if (t->get_text().size() > s.size()) {
          s = "\\ID";
          for (std::size_t i = 0; i < t->get_text().size()-s.size(); ++i) {
            s.append("_");
          }
          s.append("/");
        }
        s = pretty::ansi_format({pretty::ansi_color::BLUE}, s);
      } else if (t->is_type<floating_literal_t>()) {
        s = pretty::ansi_format({pretty::ansi_color::GREEN}, "\\FL/");
      } else if (t->is_type<integer_literal_t>()) {
        s = pretty::ansi_format({pretty::ansi_color::GREEN}, "\\IL/");
      } else if (t->is_type<integer_literal_t>()) {
        s = pretty::ansi_format({pretty::ansi_color::GREEN}, "\\SL/");
      } else {
        s = t->get_text();
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        s = pretty::ansi_format({.color = pretty::ansi_color::PURPLE, .dim = true}, s);
      }
      
      mp.print({t->get_text().append(" "),
                 s.append(" ")});
    }
  public:
    void pretty_print() {
      std::cout << mp;
    }
  private:
    using parse_result_t = lexer_result_t<token_t*>;
    
    inline void skip(char c) {
      while (input_stream.current() == c) {
        input_stream.advance();
      }
    }
    
    parse_result_t parse_token() {
      skip(' ');
      while (input_stream.current() == '\n') {
        mp.print({"\n", " "});
        input_stream.advance();
      }
      return parse_keywords()
      .or_else([&] (auto err) { return parse_literal(); })
      .or_else([&] (auto err) { return parse_symbol(); })
      .or_else([&] (auto err) { return parse_identifier(); })
      ;
    }
    
    template<keywords::keyword Keyword>
    parse_result_t parse_keyword() {
      auto mark = input_stream.mark_current();
      
      for (std::size_t i = 0; i < Keyword::text.size(); ++i, input_stream.advance()) {
        if (Keyword::text[i] != input_stream.current()) {
          input_stream.goto_marker(mark);
          return lexer_error({});
        }
      }
      
      return create_token<keyword_token_t<Keyword>>();
    }
    
    parse_result_t parse_keywords() {
      return                           parse_keyword<keywords::fn>()
      .or_else([&] (auto err) { return parse_keyword<keywords::let>(); })
      .or_else([&] (auto err) { return parse_keyword<keywords::ret>(); })
      .or_else([&] (auto err) { return parse_keyword<keywords::if_>(); })
      .or_else([&] (auto err) { return parse_keyword<keywords::else_>(); })
      .or_else([&] (auto err) { return parse_keyword<keywords::for_>(); })
      .or_else([&] (auto err) { return parse_keyword<keywords::while_>(); })
      .or_else([&] (auto err) { return parse_keyword<keywords::when>(); })
      .or_else([&] (auto err) { return parse_keyword<keywords::type>(); })
      .or_else([&] (auto err) { return parse_keyword<keywords::impl>(); })
      ;
    }
    
    parse_result_t _parse_symbol() {
      switch (input_stream.current()) {
        case '(': return create_token<symbol::left_parenthesis_t>();
        case ')': return create_token<symbol::right_parenthesis_t>();
        case '{': return create_token<symbol::left_curly_bracket_t>();
        case '}': return create_token<symbol::right_curly_bracket_t>();
        case '[': return create_token<symbol::left_square_bracket_t>();
        case ']': return create_token<symbol::right_square_bracket_t>();
        case '<': return create_token<symbol::left_angle_bracket_t>();
        case '>': return create_token<symbol::right_angle_bracket_t>();
        case '+': return create_token<symbol::plus_t>();
        case '-': return create_token<symbol::minus_t>();
        case '*': return create_token<symbol::star_t>();
        case '/': return create_token<symbol::forward_slash_t>();
        case '\\': return create_token<symbol::backward_slash_t>();
        case '=': return create_token<symbol::equal_t>();
        case '!': return create_token<symbol::negation_t>();
        case '&': return create_token<symbol::ampersand_t>();
        case '|': return create_token<symbol::pipe_t>();
        case '^': return create_token<symbol::hat_t>();
        case ':': return create_token<symbol::colon_t>();
        case ';': return create_token<symbol::semicolon_t>();
        case ',': return create_token<symbol::comma_t>();
        case '.': return create_token<symbol::dot_t>();
        default:
          return lexer_error({});
      }
    }
    parse_result_t parse_symbol() {
      return _parse_symbol().transform([&](auto t){input_stream.advance(); return t;});
    };
    
    parse_result_t parse_identifier() {
      std::string identifier;
      
      if (fits_identifier(input_stream.current()) && !is_digit(input_stream.current())) {
        identifier += input_stream.current();
        input_stream.advance();
        
        
        while (fits_identifier(input_stream.current())) {
          identifier += input_stream.current();
          input_stream.advance();
        }
      }
      
      if (identifier.empty()) {
        return lexer_error ({});
      } else {
        return create_token<identifier_token_t>(identifier);
      }
    }
    
    parse_result_t parse_number_literal() {
      std::string text;
      bool floating = false;
      
      while (input_stream.current() == '.' || is_digit(input_stream.current())) {
        text += input_stream.current();
        if (input_stream.current() == '.' ) {
          if (floating) {
            // ? Two decimal dots???
            return lexer_error ({});
          } else {
            floating = true;
          }
        }
        input_stream.advance();
      }
      
      if (text.empty()) {
        return lexer_error ({});
      }
      
      if (floating) {
        return create_token<floating_literal_t>(std::stod(text));
      } else {
        return create_token<integer_literal_t>(std::stol(text));
      }
    }
    parse_result_t parse_bool_literal() {
      return lexer_error ({});
    }
    parse_result_t parse_string_literal() {
      return lexer_error ({});
    }
    parse_result_t parse_literal() {
      return parse_number_literal()
      .or_else([&] (auto err) { return parse_string_literal(); })
      .or_else([&] (auto err) { return parse_bool_literal(); })
      ;
    }
  };
  
}

#endif //ULANG_LEXER_H
