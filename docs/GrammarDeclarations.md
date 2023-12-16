# Grammar Declarations

The declaration of a grammar starts with the tag `#[grammar: <grammar_name>]`, where `<grammar_namr>` is a unique identifier for a grammar. It is then followed by a block `{...}` where tokens and rules are specified:

```
#[grammar: example-gr] {
    ...
}
```

## Tokens

A token rule consists of an uppercase identifier followed by an equals sign and a regular expression that specifies what matches as the token.

If a token identifier begins with `_` then that token will be ignored while parsing.

```
<token-name> = /<regex>/

INT_NUM = /[0-9]+/
FLOAT_NUM = /[0-9]*\.[0-9]*/
ASSIGN = /->/
ID = /[_a-zA-Z]+[a-zA-Z0-9-_]*/
_WS = / /
_NL = /\n/
```

## Grammar Rules

