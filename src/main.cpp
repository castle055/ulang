#include <iostream>
#include <fstream>

//#include "parser/uparser.h"
//#include "parser/runtime/runtime.h"

#include "ast.h"

#include "parser.h"

template <std::size_t Lines>
static void print_lines(std::vector<std::string> lines) {
  std::array<std::string, Lines> arr{};
  for (std::size_t i = 0; i < Lines; ++i) {
    arr[i] = lines[i];
  }
  pretty::multiline_printer_t mp{.lines = arr};
  
  std::cout << mp << std::endl;
}

int main(int argc, char* argv[]) {
  ulang::file_stream_t fs{"/home/castle/repos/cpr/ulang/tests/syntax/fun_declare.ul"};
  ulang::lexer::token_stream_t ts{fs};
  
  
  while (!ts.at_end()) {
    ts.advance();
  }
  ts.pretty_print();
  
  ulang::parser_t p{fs};
  auto ast = p();
  
  auto res = print_ast{ast}();
  switch (res.size()) {
    case 1: print_lines<1>(res); break;
    case 2: print_lines<2>(res); break;
    case 3: print_lines<3>(res); break;
    case 4: print_lines<4>(res); break;
    case 5: print_lines<5>(res); break;
    case 6: print_lines<6>(res); break;
    case 7: print_lines<7>(res); break;
    case 8: print_lines<8>(res); break;
    case 9: print_lines<9>(res); break;
    case 10: print_lines<10>(res); break;
    case 11: print_lines<11>(res); break;
    case 12: print_lines<12>(res); break;
    case 13: print_lines<13>(res); break;
    case 14: print_lines<14>(res); break;
    case 15: print_lines<15>(res); break;
    case 16: print_lines<16>(res); break;
    case 17: print_lines<17>(res); break;
    case 18: print_lines<18>(res); break;
    case 19: print_lines<19>(res); break;
    case 20: print_lines<20>(res); break;
    case 21: print_lines<21>(res); break;
    case 22: print_lines<22>(res); break;
    case 23: print_lines<23>(res); break;
    case 24: print_lines<24>(res); break;
    case 25: print_lines<25>(res); break;
    case 26: print_lines<26>(res); break;
    case 27: print_lines<27>(res); break;
    case 28: print_lines<28>(res); break;
    case 29: print_lines<29>(res); break;
    case 30: print_lines<30>(res); break;
  }
  
  
  return 0;
  
  //ulang::file_t file{"../tests/loops.ul"};
  //ulang::module_t mod{file};
  //
  //auto out = mod.generate_output();
  //for (auto &item: out) {
  //  if (item == '\n') {
  //    //item = '|';
  //  }
  //}
  ////std::cout << out << std::endl;
  //
  //std::ofstream ofile{"../tests/loops.ulo"};
  //ofile << out;
  //ofile.close();
  
  //cl_boot(argc, argv);
  
  //cson_parser::parse("int a = 2; #[grammar: some-gr] { }");
  //std::ifstream file("../test.ul");
  //std::string file_s;
  //file_s.assign((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
  //std::cout << file_s << std::endl;
  //uparser::parse(file_s);
  //uparser::parse("#[grammar: ]");
  return 0;
}
//tokens {
//NUM = [0-9]+
//ID  = [a-zA-Z]+
//
//ASSIGN = "->"
//}
//rules {
//root = NUM
//}

//:init ASSIGN ID:id { int $id = $init; }

//#[grammar: id-gr] {
//tokens {
//ID  = [a-zA-Z]+
//
//TYPE_OP = "as"
//}
//rules {
//root = ID:id TYPE_OP ID:type { $id_$type }
//}
//}
//
//#[assign-gr]
//3 -> some_int
//#[assign-gr]
//4 -> #[id-gr] other as int
