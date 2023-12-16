
<h1 align="center">
  ULang
</h1>

<h4 align="center">A thing I'm having trouble describing. It's a text preprocessor, but you define macros as fully fledged grammars.</h4>

<p align="center">
<img alt="Work In Progress" src="https://img.shields.io/badge/-WIP-red?&style=for-the-badge">
<img alt="Language" src="https://img.shields.io/badge/LANG-C%2B%2B-blue?&style=for-the-badge&logo=c%2B%2B&logoColor=blue">
<img alt="GitHub" src="https://img.shields.io/github/license/castle055/cyd-ui?style=for-the-badge">
<img alt="GitHub tag (latest SemVer)" src="https://img.shields.io/github/v/tag/castle055/ulang?color=%23fcae1e&label=latest&sort=semver&style=for-the-badge">
</p>

<p align="center">
  <a href="#introduction">Introduction</a> •
  <a href="#installation">Installation</a> •
  <a href="#usage">Usage</a> •
  <a href="#credits">Credits</a> •
  <a href="#license">License</a>
</p>

# Introduction
# Installation

# Usage


## Grammars

[Grammar Declarations](docs/GrammarDeclarations.md)

```
#[grammar: assign] {
    tokens {
        INT_NUM = /[0-9]+/
        FLOAT_NUM = /[0-9]*\.[0-9]*/
        ASSIGN = /->/
        ID = /[_a-zA-Z]+[a-zA-Z0-9-_]*/
        _WS = / /
        _NL = /\n/
    }
    rules {
        assign = INT_NUM ASSIGN ID      <int $3 = $1;>
               | FLOAT_NUM ASSIGN ID    <double $3 = $1;>;
        root = assign { root }          <$1\n$2>;
    }
}
```

## Using Grammars

```
[assign|>
    5   -> a
    3   -> b
    2.5 -> f
    .3  -> e
]
```

```cpp
int a = 5;
int b = 3;
double f = 2.5;
double e = .3;
```

# Credits

This software uses the following open source projects:

- [bison]()
- [flex]()

# License

GPL 3.0

[LICENSE.md](LICENSE.md)

---

> GitHub [@castle055](https://github.com/castle055) &nbsp;&middot;&nbsp;

