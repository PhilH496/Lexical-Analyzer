//---------------------------------------------------------------------
// compiled using g++ -std=c++17 mylexer.cpp DFA.cpp NFA.cpp 
//---------------------------------------------------------------------
#include <iostream>
#include <string>
#include <stack>
#include <set>
#include <bits/stdc++.h>
#include <map>
#include "NFA.h"
#include "DFA.h"
using namespace std;

//---------------------------------------------------------------------
// define precedence and associativity of operators
//---------------------------------------------------------------------
int Precedence(char op)
{
    if (op == '*')
        return 3; // highest precedence for Kleene star (*)
    if (op == '.')
        return 2; // concatenation (.) has lower precedence than *
    if (op == '|')
        return 1; // alternation (|) has the lowest precedence
    return 0;
}

//---------------------------------------------------------------------
// check if a character is an operand (alpha)
//---------------------------------------------------------------------
bool IsOperand(char c)
{
    return isalpha(c);
}

//---------------------------------------------------------------------
// convert an infix regular expression to postfix
//---------------------------------------------------------------------
string InfixToPostfix(string infix)
{
    stack<char> ops;     // stack for operators
    string postfix = ""; // resulting postfix expression 'queue'
    for (size_t i = 0; i < infix.size(); i++)
    {
        char c = infix[i];
        if (IsOperand(c))
        {
            // if c is an operand, put it on the output queue
            postfix += c;
        }
        else if (c == '(')
        {
            // if it's an opening parenthesis, push it onto the stack
            ops.push(c);
        }
        else if (c == ')')
        {
            // pop off the stack until matching '(' is found
            while (!ops.empty() && ops.top() != '(')
            {
                postfix += ops.top();
                ops.pop();
            }
            ops.pop(); // pop the '('
        }
        else if (c == '|')
        {
            // if it's an alternation operator, pop operators of higher precedence
            while (!ops.empty() && ops.top() != '(' && Precedence(ops.top()) >= Precedence(c))
            {
                postfix += ops.top();
                ops.pop();
            }
            ops.push(c); // push the alternation operator onto the stack
        }
        else if (c == '.')
        {
            // do same for concatenation (.)
            while (!ops.empty() && ops.top() != '(' && Precedence(ops.top()) >= Precedence(c))
            {
                postfix += ops.top();
                ops.pop();
            }
            ops.push(c); // push the . operator onto the stack
        }
        else if (c == '*')
        {
            // and for (*)
            ops.push(c); // push the * operator onto the stack
        }
    }
    // pop all remaining operators off the stack
    while (!ops.empty())
    {
        postfix += ops.top();
        ops.pop();
    }
    return postfix;
}

//---------------------------------------------------------------------
// convert a regular expression in postfix to an NFA using Thompson's construction
//---------------------------------------------------------------------
NFA PostfixToNFA(string postfix)
{
    stack<NFA> nfa_stack;
    int stateCount = 0;
    for (size_t i = 0; i < postfix.size(); i++)
    {
        char c = postfix[i];
        // Creates a transition on the input character to a final state
        if (IsOperand(c)) 
        {
            int start = stateCount++;
            int end = stateCount++;
            NFA myNFA({c}, start, {end});
            myNFA.AddTransition(start, {end}, c);

            nfa_stack.push(myNFA);
        }
        else if (c == '*')
        {
            NFA myNFA = nfa_stack.top();
            nfa_stack.pop();
        
            // Create new states
            int newStart = stateCount++;
            int newFinal = stateCount++;
        
            // Create a new NFA with the same alphabet
            NFA newNFA(myNFA.getAlpha(), newStart, {newFinal});
        
            // From new start state, add ε-transitions to:
            // 1. myNFA's initial state 
            // 2. New final state 
            newNFA.AddTransition(newStart, {myNFA.getInitState(), newFinal}, '_');
        
            // Merge the NFA into the new NFA
            newNFA.Merge(myNFA);
        
            // From myNFA's final states, add ε-transitions to:
            // 1. myNFA's initial state 
            // 2. New final state 
            for (int finalState : myNFA.getFinalStates())
            {
                newNFA.AddTransition(finalState, {myNFA.getInitState(), newFinal}, '_');
            }
        
            nfa_stack.push(newNFA);
        }
        else if (c == '.')
        {
        NFA nfa2 = nfa_stack.top();
        nfa_stack.pop();
        NFA nfa1 = nfa_stack.top();
        nfa_stack.pop();

        // Add ε-transitions from nfa1's final states to nfa2's initial state
        for (int finalState : nfa1.getFinalStates())
        {
            nfa1.AddTransition(finalState, {nfa2.getInitState()}, '_');
        }

        // Merge nfa2 into nfa1
        nfa1.Merge(nfa2);

        nfa1.setFinalStates(nfa2.getFinalStates());

        nfa_stack.push(nfa1);
    }
        else if (c == '|')
        {
            NFA nfa2 = nfa_stack.top();
            nfa_stack.pop();
            NFA nfa1 = nfa_stack.top();
            nfa_stack.pop();

            nfa1.ShiftStates(1);
            nfa2.ShiftStates(stateCount - 3);

            int newStart = 0;
            int newFinal = stateCount + 1;
            stateCount = newFinal + 1;

            // Create the new union NFA.
            NFA newNFA(nfa1.getAlpha(), newStart, {newFinal});

            // From the new start state, add ε-transitions to both NFAs initial states.
            newNFA.AddTransition(newStart, {nfa1.getInitState(), nfa2.getInitState()}, '_');

            // Merge the two NFAs into newNFA.
            newNFA.Merge(nfa1);
            newNFA.Merge(nfa2);

            // For each final state in nfa1/2, add an ε-transition to the new final state.
            for (int finalState : nfa1.getFinalStates())
            {
                newNFA.AddTransition(finalState, {newFinal}, '_');
            }
            for (int finalState : nfa2.getFinalStates())
            {
                newNFA.AddTransition(finalState, {newFinal}, '_');
            }

            nfa_stack.push(newNFA);
        }
    }
    return nfa_stack.top();
}

//---------------------------------------------------------------------
// convert a NFA to DFA using subset construction
//---------------------------------------------------------------------
DFA NFAtoDFA(NFA myNFA)
{
    map<int, map<char, set<int>>> Ntran = myNFA.getNFATransitions();
    int NFAStart = myNFA.getInitState();
    set<int> NFAFinals = myNFA.getFinalStates();
    set<char> alpha = myNFA.getAlpha();

    map<set<int>, map<char, set<int>>> Dtran; // DFA transition map
    map<set<int>, int> stateMapping;          // Maps NFA states to its corresponding DFA state
    queue<set<int>> unmarkedStates;           // States yet to be processed
    set<set<int>> Dstates;                    // DFA states

    // Create initial DFA state from NFA initial state 
    set<int> startSet = {NFAStart};
    set<int> startClosure = myNFA.EpsilonClosure(startSet);
    Dstates.insert(startClosure);
    unmarkedStates.push(startClosure);
    stateMapping[startClosure] = 0;

    int stateCount = 0;
    DFA myDFA(alpha, {0}, {});

    // Process all unmarked states
    while (!unmarkedStates.empty())
    {
        set<int> T = unmarkedStates.front();
        unmarkedStates.pop();

        // For each input symbol
        for (char a : alpha)
        {
            if (a == '_') { // Skip epsilon transitions
                continue;
            }

            set<int> U = myNFA.EpsilonClosure(myNFA.move(T, a)); // Get states reachable by the input symbol(using move(T,a)) then by epsilon
            if (!U.empty())
            {
                if (Dstates.count(U) == 0) // If the DFA set isn't in our set of DFAs, add it to be processed
                {
                    Dstates.insert(U);
                    unmarkedStates.push(U);
                    stateCount++;
                    stateMapping[U] = stateCount;
                }
                Dtran[T][a] = U;
            }
        }
    }

    // Convert state set transitions to DFA state numbers
    for (auto &row : Dtran)
    {
        int src = stateMapping[row.first];
        for (auto &trans : row.second)
        {
            int dst = stateMapping[trans.second];
            myDFA.AddTransition(src, dst, trans.first);
        }
    }

    set<int> DFAFinals;
    for (auto &entry : stateMapping)
    {
        const set<int> &stateSet = entry.first;
        int dfaStateID = entry.second;
        // For each final NFA state, make the DFA state final 
        for (int nfaState : stateSet)
        {
            if (NFAFinals.find(nfaState) != NFAFinals.end())
            {
                DFAFinals.insert(dfaStateID);
                break;
            }
        }
    }
    myDFA.setFinalStates(DFAFinals);
    return myDFA;
}

//--------------------------------------------------------------
// class Token
//--------------------------------------------------------------
class Token
{
public:
    Token(string t, string v) : type(t), value(v) {}
    Token() : type("EOS"), value("") {}
    string type;  
    string value;
};

//--------------------------------------------------------------
// class Lexer
//--------------------------------------------------------------
class Lexer
{
public:
    Lexer(const string &input, map<string, DFA> &dfas) : input(input), pos(0), tokens(dfas) {}
    Token getToken();

private:
    string input;
    size_t pos;
    map<string, DFA> tokens; // Map of token names to their DFAs
};

//--------------------------------------------------------------
// returns a token by matching the input against the token DFAs
// an token is INVALID if no DFA accepts the sequence
//--------------------------------------------------------------
Token Lexer::getToken()
{
    // Reset all DFAs
    for (auto &tokenDFA : tokens)
    {
        tokenDFA.second.Reset();
    }

    // Skip whitespace
    while (pos < input.size() && isspace(input[pos]))
        pos++;

    if (pos == input.size())
        return Token("EOS", "");

    size_t start_pos = pos;
    string best_match = "";         // Longest accepted string found
    string best_token = "";         // Token associated with longest string
    size_t best_end_pos = pos;      // Position after longest match
    bool has_match = false;         // If we found a matching string
    bool all_dead = false;          // If all DFAs have reached dead states

    while (pos < input.size() && !isspace(input[pos]) && !all_dead)
    {
        char c = input[pos];
        all_dead = true;  // Assume all DFAs will die until proven otherwise

        // Try each token's DFA
        for (auto &tokenDFA : tokens)
        {
            if (!tokenDFA.second.IsDead())  // Only move if DFA isn't dead
            {
                tokenDFA.second.Move(c);    
                if (!tokenDFA.second.IsDead()) // If any DFA is still alive then we continue
                {
                    all_dead = false; 
                }
                
                // Check if the accepted string is longest match
                if (tokenDFA.second.GetAccepted())
                {
                    string lexeme = tokenDFA.second.GetAcceptedLexeme();
                    if (lexeme.length() > best_match.length())
                    {
                        best_match = lexeme;
                        best_token = tokenDFA.first;
                        best_end_pos = pos + 1;
                        has_match = true;
                    }
                }
            }
        }
        pos++;
    }

    if (has_match)
    {
        pos = best_end_pos;
        return Token(best_token, best_match);
    }

    // Return invalid token if no match found
    return Token("INVALID", string(1, input[start_pos++]));
}

//---------------------------------------------------------------------
// read token definitions until we encounter the given terminator
//---------------------------------------------------------------------
string readUntilTerminator(istream &in, char terminator)
{
    string result;
    char c;
    while (in.get(c)) {
        if (c == terminator) {
            break;
        }
        result += c;
    }
    return result;
}

//---------------------------------------------------------------------
// gets string inside quotes
//---------------------------------------------------------------------
string readQuotedString(istream &in)
{
    string result;
    char c;

    // Find opening quote
    while (in.get(c) && c != '"') {} // Skip characters until the opening quote

    // Read until closing quote
    while (in.get(c) && c != '"') {
        result += c;
    }
    return result;
}

//---------------------------------------------------------------------
// trim leading and trailing whitespace characters
//---------------------------------------------------------------------
string trim(const string &str) {
    // Find first non-whitespace character
    size_t start = str.find_first_not_of(" \t\n");
    if (start == string::npos) {
        return ""; // String consists only of whitespace
    }

    // Find last non-whitespace character
    size_t end = str.find_last_not_of(" \t\n");

    // Return the trimmed substring
    return str.substr(start, end - start + 1);
}

//---------------------------------------------------------------------
// split string based on input character
//---------------------------------------------------------------------
vector<string> split(string input, char delimeter) {
    stringstream ss(input);
    vector<string> tokens;
    string word;
    while (!ss.eof())
    {
        getline(ss, word, delimeter);
        tokens.push_back(word);
    }
    return tokens;
}

//---------------------------------------------------------------------
// helper method that calls the other methods to convert regex to DFA
//---------------------------------------------------------------------
DFA regexToDFA(string infix) {
    string postfix = InfixToPostfix(infix);
    NFA resultNFA = PostfixToNFA(postfix);
    DFA resultDFA = NFAtoDFA(resultNFA);
    return resultDFA;
}

int main()
{
    // Read token definitions from standard input
    auto &in = cin;

    // Read token definitions until '#'
    string tokenDefsString = readUntilTerminator(in, '#');

    // Split token definitions
    vector<string> tokensDefinitions = split(tokenDefsString, ',');
    map<string, DFA> tokens;
    set<string> emptyAcceptingTokens;

    // Process each token definition after trimming whitespace
    for (string& tokenDef : tokensDefinitions) {
        tokenDef = trim(tokenDef); // Remove leading and trailing whitespace
        vector<string> splitTokens = split(tokenDef, ' '); // Split regex from its token name
        string tokenName = splitTokens[0];
        // Reconstruct the regex
        string regex;
        for (size_t i = 1; i < splitTokens.size(); ++i) {
            if (i > 1) regex += " ";
            regex += splitTokens[i];
        }
            
        // Create DFA from regex
        DFA dfa = regexToDFA(regex);
        tokens[tokenName] = dfa;
        if (dfa.acceptsEmptyString()) { // Check if the initial state is the final state
            emptyAcceptingTokens.insert(tokenName);
        }
    }

    // Check if the initial state is the accepting state
    if (!emptyAcceptingTokens.empty()) {
        cout << "EPSILON IS NOT A TOKEN";
        for (auto& tokenName : emptyAcceptingTokens) {
            cout << " " << tokenName;
        }
        cout << endl;
        exit(1);
    }

    // Read the input string by checking for starting and ending quotes
    string input_string = readQuotedString(in);

    // Second line contains input string to tokenize
    if (input_string.front() == '"' && input_string.back() == '"')
    {
        input_string = input_string.substr(1, input_string.length() - 2);
    }

    // Create lexer with input string and token DFAs
    Lexer lexer(input_string, tokens);

    // Get all tokens
    Token token;
    while ((token = lexer.getToken()).type != "EOS")
    {
        if (token.type == "INVALID")
        {
            cout << "ERROR" << endl;
            break;
        }
        else
        {
            cout << token.type << " , \"" << token.value << "\"" << endl;
        }
    }
    return 0;
}