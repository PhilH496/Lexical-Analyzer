#ifndef NFA_H
#define NFA_H

#include <iostream>
#include <set>
#include <map>
#include <queue>
#include <stack>
#include <string>
using namespace std;

//---------------------------------------------------------------------------------
// class NFA
// (S, Σ, δ, s0, F)
//---------------------------------------------------------------------------------
class NFA
{
public:
    NFA(set<char> A, int I, set<int> F) : alphabet(A), init_state(I), fin_states(F) {}
    void AddTransition(int src, set<int> dst, char sym) { Ntran[src][sym] = dst; }
    set<char> getAlpha() const { return alphabet; }
    int getInitState() const { return init_state; }
    set<int> getFinalStates() const { return fin_states; }
    void setFinalStates(const set<int> &newFinalStates) { fin_states = newFinalStates; }
    map<int, map<char, set<int>>> getNFATransitions() const { return Ntran; }
    void Print();
    set<int> EpsilonClosureState(int state) const;  
    set<int> EpsilonClosure(const set<int>& states) const;  
    set<int> move(const set<int>& T, char symbol) const;
    void ShiftStates(int offset);
    void Merge(const NFA &other);

private:
    map<int, map<char, set<int>>> Ntran; // NFA transitions
    set<char> alphabet;  // set of input symbols for a NFA
    int init_state;      
    set<int> fin_states; 
};

#endif