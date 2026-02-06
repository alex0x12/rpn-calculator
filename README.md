# RPN Calculator

A command-line calculator that parses infix expressions with the **Shunting Yard** algorithm and evaluates them using **Reverse Polish Notation (RPN)**.
It's expected to provide grammatically valid expression as there's no preprocessing yet 

## Features
- Infix to RPN conversion + evaluation
- Basic arithmetic: `+`, `-`, `*`, `/`, `^`
- Unary operators: percent `%`, factorial `!`, unary minus
- Basic functions: `sin`, `cos`, `tan`, `cot`, `ln`,
- `log`, `exp`, `sqrt`, `min`, `max`
- Constants: `p`(π), `e`

## Usage
```
./bin/rpn [options] -- 'expr1' 'expr2' ...
```
You must provide at least 1 expression string!

### Options
- `-p`, `--precision <n>`: number of decimal places (default: 3, max: 30)
- `-b`, `--brief`: print only the result (no labels)

The `--` separator prevents expressions that start with `-` from being treated as options.

## Operator Precedence
From highest to lowest:
1. Unary operators: `~` (unary minus), `%`, `!`
2. Power: `^` (right-associative)
3. Multiplication/Division: `*`, `/`
4. Addition/Subtraction: `+`, `-`

Parentheses `()` override precedence.

## Percent Rules
- `%` is unary: `x%` means `x/100`.
- In expressions of the form `A + B%` and `A - B%`, the percent is treated as a fraction of the left operand:
- `A + B%` → `A * (1 + B/100)`
- `A - B%` → `A * (1 - B/100)`
- Example: `50%` → `0.5`, `200+10%` → `220`.

## Usage example
```
rpn -p 5 -b -- '1+2+3' 'log(2,8)' '200+10%' 'sqrt(min(log(8,64),64))'
```
## Build

Debug (default):
```
./build.sh
```
Release:
```
./build.sh -release
```
Result will be placed in `bin/rpn`.

## Tests
`tests.sh` validates:
- arithmetic and precedence,
- functions and constants,
- percents and parentheses,
- unary operations,
- multiple precision values.

Run:
```
./tests.sh
```

## Output
By default, the program prints:
- `RPN:` - the Reverse Polish Notation form
- `Res:` - the final result

With `-b`, it prints only the numeric result.

# TODO 
- `Preprocessing`
- `Bitwise operations`
- `Interactive mode`
- `Comments`
