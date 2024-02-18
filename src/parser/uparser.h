//
// Created by castle on 7/24/23.
//

#ifndef UPARSER_H
#define UPARSER_H

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <regex>

namespace ulang {
  using str = std::string;
  
  struct keywords {
    keywords() = delete;
    
    static constexpr const char* DEF = "def";
  };
  
  struct char_is {
    char_is() = delete;
    
    static inline bool digit(char c) {
      return c == '0'
        || c == '1'
        || c == '2'
        || c == '3'
        || c == '4'
        || c == '5'
        || c == '6'
        || c == '7'
        || c == '8'
        || c == '9';
    }
    static inline bool alpha(char c) {
      return c == 'a'
        || c == 'b'
        || c == 'c'
        || c == 'd'
        || c == 'e'
        || c == 'f'
        || c == 'g'
        || c == 'h'
        || c == 'i'
        || c == 'j'
        || c == 'k'
        || c == 'l'
        || c == 'm'
        || c == 'n'
        || c == 'o'
        || c == 'p'
        || c == 'q'
        || c == 'r'
        || c == 's'
        || c == 't'
        || c == 'u'
        || c == 'v'
        || c == 'w'
        || c == 'x'
        || c == 'y'
        || c == 'z';
    }
    static inline bool allowed_in_identifier(char c) {
      return digit(c)
        || alpha(c)
        || c == '_'
        || c == '$'
        || c == '-'
        || c == '+'
        ;
    }
  };
  
  struct definition_t {
    enum class parameter_type_e {
      STRING,
      NUMBER,
      BLOCK
    };
    
    struct overload_t {
      std::vector<std::pair<str, parameter_type_e>> parameters;
      str pattern;
      
      [[nodiscard]]
      bool matches(const std::vector<parameter_type_e>& p_types) const {
        if (p_types.size() != parameters.size()) return false;
        for (int i = 0; const auto& pt: p_types) {
          if (parameters[i++].second != pt) return false;
        }
        return true;
      }
      
      [[nodiscard]]
      str apply(const std::vector<str>& params) const {
        // IT IS ASSUMED THAT `matches()` RETURNS TRUE FOR THIS OVERLOAD
        for (std::size_t i = 0; const auto& [id, ptype]: parameters) {
          const str& param = params[i++];
          
        }
        
        str payload;
        str param_id;
        unsigned long i = 0;
        while (i < pattern.size()) {
          if (i < pattern.size()-1 && pattern[i] == '$' && pattern[i] != '[') {
            param_id.clear();
            i++;
            
            while (char_is::allowed_in_identifier(pattern[i])) {
              param_id += pattern[i];
              i++;
            }
            
            std::size_t p_id = 0;
            for (const auto &[id, _]: parameters) {
              if (id == param_id) {
                payload.append(params[p_id]);
                break;
              }
              p_id++;
            }
          } else {
            payload += pattern[i];
            i++;
          }
        }
        
        return payload;
      }
    };
    
    str identifier;
    std::vector<overload_t> overloads{};
  };
  
  class file_t {
    str text;
  public:
    const std::string path;
    
    explicit file_t(const std::string& path)
      : path(path) {
      std::ifstream file(path);
      text.assign((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
      file.close(); // Is this necessary?
    }
    
    [[nodiscard]]
    str as_str() const {
      return text;
    }
  };

#define SAVE_CHECKPOINT unsigned long checkpoint = index
#define GOTO_CHECKPOINT index = checkpoint
  
  class annotated_source_t {
    str payload;
    std::unordered_map<str, definition_t> definitions{};
    
    // Parser?
    str text{};
    unsigned int index = 0;
    
    bool is_EOF() const {
      return index >= text.size();
    }
    char current() const {
      if (index < text.size()) {
        return text[index];
      } else {
        return '\0';
      }
    }
    void accept(unsigned long n = 1) {
      index += n;
    }
    void accept_as_payload(unsigned long n = 1) {
      payload.append(text.substr(index, n));
      accept(n);
    }
    void skip_all(char c) {
      while (!is_EOF() && current() == c) {
        accept();
      }
    }
    void skip_all(std::unordered_set<char> cs) {
      while (!is_EOF() && cs.contains(current())) {
        accept();
      }
    }
    
    bool parse_identifier(str& id) {
      while (char_is::allowed_in_identifier(current())) {
        id += current();
        accept();
      }
      return !id.empty();
    }
    
    bool parse_payload(str& pload, bool multiline) {
      char end_char = multiline? '}': '\n';
      bool escaped_char = false;
      while (current() != end_char) {
        if (is_EOF()) {
          return true;
        }
        if (current() == '\\') {
          escaped_char = true;
          accept();
        }
        
        SAVE_CHECKPOINT;
        if (!escaped_char && current() == '$' && text[index+1] == '[') {
          accept(2);
          if (!parse_substitution()) {
            GOTO_CHECKPOINT;
          }
        } else {
          pload += current();
          accept();
        }
      }
      accept();
      return true;
    }
    
    bool parse_parameters(std::vector<std::pair<str, definition_t::parameter_type_e>>& params) {
      accept();
      str param_id{};
      while (current() != ')') {
        skip_all({' ', '\n'});
        if (is_EOF()) {
          return false;
        }
        if (!parse_identifier(param_id)) {
          return false;
        }
        // TODO - Parse param type
        skip_all({' ', '\n'});
        if (current() == ',') {
          params.emplace_back(param_id, definition_t::parameter_type_e::STRING);
          param_id.clear();
          accept();
        } else {
          // end of params
          break;
        }
      }
      if (current() == ')') {
        params.emplace_back(param_id, definition_t::parameter_type_e::STRING);
        accept();
        return true;
      } else {
        // params ended with something that wasn't ")"
        return false;
      }
    }
    
    bool parse_single_overload(definition_t::overload_t& ov) {
      accept();
      skip_all({' ', '\n'});
      str pload{};
      if (current() == '{') {
        accept();
        
        // Skip just the right amount of spaces and newlines when appropriate
        SAVE_CHECKPOINT;
        skip_all(' ');
        if (current() == '\n') accept();
        else GOTO_CHECKPOINT;
        
        if(!parse_payload(pload, true)) {
          return false;
        }
      } else {
        if(!parse_payload(pload, false)) {
          return false;
        }
      }
      if (pload[pload.size()-1] == '\n') {
        pload = pload.substr(0, pload.size()-1);
      }
      ov.pattern = pload;
      return true;
    }
    
    bool parse_overload_block(definition_t& def) {
      accept();
      skip_all({' ', '\n'});
      while (current() != '}') {
        if (current() == '(') {
          std::vector<std::pair<str, definition_t::parameter_type_e>> parameters;
          if (!parse_parameters(parameters)) {
            return false;
          }
          skip_all({' ', '\n'});
          if (current() == '-') {
            accept();
            if (current() != '>') {
              return false;
            }
            definition_t::overload_t ov{};
            if (!parse_single_overload(ov)) {
              return false;
            }
            ov.parameters = parameters;
            def.overloads.push_back(ov);
          } else {
            // found a param list without a payload after it
            return false;
          }
        } else if (current() == '-') {
          accept();
          if (current() != '>') {
            return false;
          }
          definition_t::overload_t ov{};
          if (!parse_single_overload(ov)) {
            return false;
          }
          def.overloads.push_back(ov);
        } else {
          // Empty of invalid overload block
          return false;
        }
        skip_all({' ', '\n'});
      }
      accept();
      return true;
    }
    
    bool parse_definition_payload(definition_t& def) {
      skip_all(' ');
      if (current() == '{') {
        if (!parse_overload_block(def)) {
          return false;
        }
      } else if (current() == '(') {
        std::vector<std::pair<str, definition_t::parameter_type_e>> parameters;
        if (!parse_parameters(parameters)) {
          return false;
        }
        skip_all({' ', '\n'});
        if (current() == '-') {
          accept();
          if (current() != '>') {
            return false;
          }
          definition_t::overload_t ov{};
          if (!parse_single_overload(ov)) {
            return false;
          }
          ov.parameters = parameters;
          def.overloads.push_back(ov);
        } else {
          // found a param list without a payload after it
          return false;
        }
      } else if (current() == '-') {
        accept();
        if (current() != '>') {
          return false;
        }
        definition_t::overload_t ov{};
        if (!parse_single_overload(ov)) {
          return false;
        }
        def.overloads.push_back(ov);
      } else {
        // Do nothing, this def is a 'flag' and has no payloads
      }
      definitions[def.identifier] = def;
      return true;
    }
    
    bool parse_definition() {
      skip_all(' ');
      definition_t def{};
      while (!is_EOF() && current() != ']' && current() != ' ' && current() != '\n') {
        def.identifier += current();
        accept();
      }
      skip_all({' ', '\n'});
      if (current() == ']') {
        accept();
        if (!parse_definition_payload(def)) {
          return false;
        }
      } else {
        return false;
      }
      return true;
    }
    
    // ? Substitution Parser
    
    bool parse_substitution_params(std::vector<str>& params) {
      str param{};
      unsigned int parenthesis_count = 1;
      accept(); // accept `(`
      while (!is_EOF()) {
        skip_all({' ', '\n'});
        if (current() == '(') {
          parenthesis_count++;
        } else if (current() == ')') {
          parenthesis_count--;
          if (parenthesis_count == 0) {
            if (!param.empty()) {
              params.push_back(param);
            }
            accept();
            return true;
          }
        }
        if (current() == ',') {
          if (!param.empty()) {
            params.push_back(param);
          }
          param.clear();
        } else {
          param += current();
        }
        accept();
      }
      if (is_EOF()) {
        return false;
      }
      return false;
    }
    
    bool parse_substitution() {
      str id;
      std::vector<str> params{};
      unsigned long start_i = index -2; // Accounting for `#[` or `$[`
      while (!is_EOF() && current() != ']' && current() != '(' && current() != ' ' && current() != '\n') {
        id += current();
        accept();
      }
      skip_all({' ', '\n'});
      if (current() == '(') {
        if (!parse_substitution_params(params)) {
          return false;
        }
        //accept();
        skip_all({' ', '\n'});
      }
      if (current() != ']') {
        return false;
      }
      accept();
      
      std::vector<definition_t::parameter_type_e> p_types;
      for (const auto &item: params) {
        p_types.push_back(definition_t::parameter_type_e::STRING);
      }
      
      if (!definitions.contains(id)) return false;
      if (definitions[id].overloads.empty()) return false;
      bool found = false;
      for (const auto &ov: definitions[id].overloads) {
        if (ov.matches(p_types)) {
          found = true;
          str substitution = ov.apply(params);
          str prefix = text.substr(0,start_i);
          str suffix = text.substr(index);
          text.clear();
          text.append(prefix);
          text.append(substitution);
          text.append(suffix);
          index = start_i;
          break;
        }
      }
      
      return found;
    }
    
    bool parse_expression() {
      accept();
      if (current() == '[') {
        accept();
      } else {
        return false;
      }
      skip_all(' ');
      if ( text[index]   == keywords::DEF[0]
        && text[index+1] == keywords::DEF[1]
        && text[index+2] == keywords::DEF[2]
        && text[index+3] == ':') {
        accept(4);
        if (!parse_definition()) {
          return false;
        }
        skip_all({' ', '\n'});
      } else {
        // TODO - parse definition call
        if (!parse_substitution()) {
          return false;
        }
      }
      return true;
    }
    
    void reset_parser() {
      index = 0;
      definitions.clear();
      payload.clear();
    }
    void parse(const str& file) {
      reset_parser();
      text = file;
      while (!is_EOF()) {
        if (current() == '#') {
          SAVE_CHECKPOINT;
          if (!parse_expression()) {
            GOTO_CHECKPOINT;
            accept_as_payload();
          }
        } else {
          accept_as_payload();
        }
      }
    }
  public:
    explicit annotated_source_t(const str& file) {
      parse(file);
    }
    
    std::string generate_output() {
      return payload;
    }
  };
  
  class module_t {
    std::unordered_map<std::string, annotated_source_t*> sources{};
    std::string                                          main_source;
  
  public:
    explicit module_t(const file_t& main_file)
      : main_source(main_file.path) {
      add_file(main_file);
    }
    ~module_t() {
      for (auto &[path, file]: sources) {
        delete file;
      }
    }
    
    void add_file(const file_t& file) {
      if (!sources.contains(file.path)) {
        sources[file.path] = new annotated_source_t{file.as_str()};
      }
    }
    
    std::string generate_output() {
      return sources[main_source]->generate_output();
    }
  };
  
}

namespace uparser {
  void parse(const std::string& str);
}

#endif //UPARSER_H
