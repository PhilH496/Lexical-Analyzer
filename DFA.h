#ifndef DFA_H
#define DFA_H

#include <set>
#include <map>

using namespace std;

enum DFAstatus {START, FAIL, POTENTIAL, ACCEPT};

//---------------------------------------------------------------------------------
// class DFA
// (S, Σ, δ, s0, F)
//---------------------------------------------------------------------------------
class DFA 
{
public:
    DFA() {}
    DFA(set<char> A, set<int> I, set<int> F);
    void Reset();
    void AddTransition( int src, int dst, char sym) {Dtran[src][sym] = dst;}
    void setFinalStates ( const set<int> &newFinalStates) { fin_states = newFinalStates; }
    void Move( char c);
    DFAstatus GetStatus() {return status;}
    string GetAcceptedLexeme() {return accepted_lexeme;}
    bool GetAccepted() {return accepted;}
    void Print();
    bool IsDead() const { return status == FAIL; }
    bool acceptsEmptyString() const;

private:
    map< int, map<char, int> > Dtran;
    DFAstatus status;
    int current_state;
    bool accepted;
    string lexeme;
    string accepted_lexeme;
    set<int> init_states; // initial state of the DFA
    set<int> fin_states;  // final states of the DFA
};

#endif