# RegEx-Based Lexical Analyzer

A configurable lexical analyzer that automatically constructs tokenizers from regular expressions. This tool implements Thompson's construction algorithm to convert regular expressions to NFAs, then uses the subset construction method to convert NFAs to DFAs, creating an efficient token recognition system.

## Features

- Converts regular expressions to postfix notation
- Implements Thompson's construction to build NFAs from regular expressions
- Converts NFAs to optimized DFAs using subset construction
- Supports Kleene star (*), concatenation (.), and alternation (|) operations
- Configurable token definitions through a simple interface
- Validates that no token accepts an empty string

## How It Works

1. Define tokens with regular expressions
2. The system automatically constructs a DFA for each token
3. Input strings are processed through all DFAs simultaneously
4. The longest matching token is selected at each position
5. Invalid tokens are properly detected and reported
