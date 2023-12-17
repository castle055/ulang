#include <utility>

//
// Created by castle on 9/17/23.
//

#ifndef ULANG_PATTERN_PARSER_H
#define ULANG_PATTERN_PARSER_H

struct pattern_result {
  std::string str;
  std::vector<std::pair<std::size_t, std::string>> parameters{};
  
  std::string apply(std::vector<std::string> params) const {
    std::string res = str;
    if (!parameters.empty()) {
      for (std::size_t i = std::max(params.size(), parameters.size())-1;; --i) {
        std::string tmp = res.substr(0, parameters[i].first);
        if (params.size() > i) {
          tmp.append(params[i]);
        }
        tmp.append(res.substr(parameters[i].first));
        res = tmp;
        
        if (i == 0) break; // bc `i` is of unsigned type and will never not be >=0
      }
    }
    return res;
  }
};

struct pattern_parser {
  std::string pattern;
  explicit pattern_parser(std::string _pattern) {
    pattern = std::move(_pattern);
    if (!pattern.empty() && pattern[0] == '\n') {
      pattern = pattern.substr(1);
    }
    if (!pattern.empty() && pattern[pattern.size()-1] == '\n') {
      pattern = pattern.substr(0, pattern.size()-1);
    }
    pattern = remove_const_indentation(pattern);
  }
  
  pattern_result parse(std::vector<pattern_result> _inputs) {
    inputs = std::move(_inputs);
    results.emplace();
    handle_pattern();
    return results.top();
  }
private:
  static std::string remove_const_indentation(const std::string& str) {
    std::size_t width = 0;
    while (str[width] == ' ') width++;
    
    std::string tmp = str.substr(width);
    std::size_t i = 0;
    while (i < tmp.size()) {
      if (tmp[i++] == '\n') {
        std::size_t w = 0;
        while (tmp[i+w] == ' ') w++;
        if (w > width) {
          w = width;
        }
        
        std::string s = tmp.substr(0,i);
        s.append(tmp.substr(i+w));
        tmp = s;
      }
    }
    return tmp;
  }
  
  std::stack<pattern_result> results{};
  
  std::vector<pattern_result> inputs{};
  //std::stack<> parse_stack{};
  
  [[nodiscard]]
  inline char current() const { return pattern[off]; }
  inline void advance(std::size_t n = 1) { off += n; }
  inline void backtrack(std::size_t n = 1) { off -= n; }
  inline void accept() {
    results.top().str += pattern[off];
    advance();
  }
  
  void handle_pattern() {
    while (off < pattern.size()) {
      switch (current()) {
        case '$':
          handle_substitution();
          break;
        case '%':
          accept();
          break;
        case '\n':
          accept();
          break;
        case '\\':
          accept();
          break;
        default:
          accept();
          break;
      }
    }
  }
  
  void handle_substitution() {
    advance();
    
    switch (char c = current()) {
      case '[':
        if (!handle_param_placeholder()) {
          //? Dollar sign meant NOTHING, just accept it and ignore it
          backtrack();
          accept();
        }
        break;
      case '{':
        break;
      default:
        if (::isdigit(c)) {
          handle_direct_sub();
        } else {
          //? Dollar sign meant NOTHING, just accept it and ignore it
          backtrack();
          accept();
        }
        break;
    }
  }
  
  bool handle_param_placeholder() {
    advance();
    
    if (current() == ']') {
      results.top().parameters.emplace_back(results.top().str.size(), "");
      advance();
      return true;
    } else {
      backtrack();
      return false;
    }
  }
  
  void handle_direct_sub() {
    std::string num_s;
    while (::isdigit(current())) {
      num_s += current();
      advance();
    }
    std::size_t index = std::stol(num_s);
    if (index > 0) --index;
    
    std::vector<std::string> params{};
    if (current() == '[') {
      params = handle_direct_sub_params();
    }
    
    std::string substitution = inputs[index].apply(params);
    
    if (current() == '{') {
      // TODO - perform eval
    }
    
    if (inputs.size() > index) {
      results.top().str.append(substitution);
    }
  }
  
  std::vector<std::string> handle_direct_sub_params() {
    advance();
    
    std::vector<std::string> params{};
    
    while (current() != ']') {
      results.emplace();
      while (current() != ',' && current() != ']') {
        switch (current()) {
          case '$':
            handle_substitution();
            break;
          case '\\':
            advance();
            break;
          default:
            accept();
            break;
        }
      }
      auto res = results.top();
      results.pop();
      params.push_back(res.apply({}));
      if (current() == ',') {
        advance();
      }
    }
    
    if (current() == ']') {
      advance();
    }
    return params;
  }
  
  std::size_t off = 0;
};

#endif //ULANG_PATTERN_PARSER_H
