//
// Created by castle on 2/16/24.
//

#ifndef ULANG_PRETTY_PRINTER_H
#define ULANG_PRETTY_PRINTER_H

#include <iostream>
#include <string>
#include <array>

namespace pretty {
  enum class ansi_color {
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    BLUE = 4,
    PURPLE = 5,
    CYAN = 6,
    WHITE = 7,
  };
  struct ansi_format_t {
    ansi_color color;
    ansi_color bkg_color;
    bool bold = false;
    bool dim = false;
    bool hi_intensity = false;
  };
  inline std::string ansi_format(ansi_format_t format, std::string text) {
    text.append("\033[0m");
    std::string prefix;
    prefix.append("\033[");
    prefix.append(format.bold? "1;": (format.dim? "2;":"0;"));
    prefix.append(format.hi_intensity? std::to_string((int)format.color+90):std::to_string((int)format.color+30));
    prefix.append("m");
    
    prefix.append(text);
    return prefix;
  }
  inline std::string strip_null_char(std::string str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
      if (str[i] == '\x00') {
        std::size_t null_ends = i;
        while (str[null_ends] == '\x00' && null_ends < str.size()) {
          null_ends++;
        }
        std::string tmp = i==0? "": str.substr(0, i);
        if (null_ends < str.size()) {
          tmp.append(str.substr(null_ends));
        }
        str = tmp;
      }
    }
    return str;
  }
  inline std::string strip_ansi_format(std::string str) {
    for (std::size_t i = 0; i < str.size()-1; ++i) {
      if (str[i] == '\033' && str[i+1] == '[') {
        std::size_t code_ends = i+1;
        while (str[code_ends] != 'm' || code_ends >= str.size()) {
          code_ends++;
        }
        std::string tmp = i==0? "": str.substr(0, i);
        if (code_ends+1 < str.size()) {
          tmp.append(str.substr(code_ends+1));
        }
        str = tmp;
      }
    }
    return str;
  }
  inline std::string get_padding_str(std::size_t len) {
    std::string s;
    s.resize(len, ' ');
    return s;
  }
  
  template <std::size_t Lines = 1>
  struct multiline_printer_t {
    void print(std::array<std::string, Lines> _lines) {
      std::size_t max_len = 0;
      std::size_t invisible_max_len = 0;
      for (const auto &l: _lines) {
        std::size_t stripped_size = strip_ansi_format(l).size();
        max_len = std::max(max_len, stripped_size);
        invisible_max_len = std::max(max_len, l.size() - stripped_size);
      }
      
      for (std::size_t i = 0; i < lines.size(); ++i) {
        if (i > 0) {
          // ? Only first line can have newlines
          for (std::size_t j = 0; j < _lines[i].size(); ++j) {
            if (_lines[i][j] == '\n') {
              _lines[i][j] = ' ';
            }
          }
        }
        std::string invis_padding;
        invis_padding.resize(invisible_max_len - (_lines[i].size() - strip_ansi_format(_lines[i]).size()),
                             '\x00');
        lines[i].append(invis_padding);
        lines[i].append(_lines[i]);
        lines[i].append(get_padding_str(max_len - strip_ansi_format(_lines[i]).size()));
      }
    }
    
    std::array<std::string, Lines> lines;
  };
  
}

template <std::size_t Lines>
inline std::ostream &operator<<(std::ostream &o, const pretty::multiline_printer_t<Lines>& mp) {
  std::size_t current_index = 0UL;
  while (current_index < mp.lines[0].size()) {
    std::size_t next_newline = mp.lines[0].find_first_of('\n', current_index);
    
    std::string first_line = mp.lines[0].substr(current_index, next_newline-current_index);
    std::size_t line_len = first_line.size() +1; // ? Account for newline character too
    
    o << "╭ " << pretty::strip_null_char(first_line) << std::endl;
    for (std::size_t i = 1; i < mp.lines.size(); ++i) {
      std::string sub_line = mp.lines[i].substr(current_index, line_len);
      o << (i==mp.lines.size()-1? "╰ ":"│ ") << pretty::strip_null_char(sub_line) << std::endl;
    }
    
    current_index += line_len;
  }
  
  return o;
}

#endif //ULANG_PRETTY_PRINTER_H
