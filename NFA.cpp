#include <iostream>
#include <set>
#include <map>
#include <queue>
#include <stack>
#include <string>
using namespace std;
#include "NFA.h"

//---------------------------------------------------------------------------------
// print the NFA
//---------------------------------------------------------------------------------
void NFA::Print()
{
    cout << "NFA Transitions:\n";
    for (const auto &nfa_row : Ntran)
    {
        cout << nfa_row.first << ":\t";
        for (const auto &transition : nfa_row.second)
        {
            cout << transition.first << ": { ";
            for (int state : transition.second)
                cout << state << " ";
            cout << "} ";
        }
        cout << endl;
    }
}

//---------------------------------------------------------------------------------
// epsilon closure of a single state in the NFA
// a dfs traversal of epsilon transitions
//---------------------------------------------------------------------------------
set<int> NFA::EpsilonClosureState(int state) const {
    set<int> epsilon_closure;
    stack<int> stack;
    
    epsilon_closure.insert(state);
    stack.push(state);

    while (!stack.empty()) {
        int curr = stack.top(); stack.pop();

        // Check if there are any epsilon transitions from current state
        auto iterator = Ntran.find(curr);
        if (iterator != Ntran.end()) {
            auto epsilon_transition = iterator->second.find('_'); // Try to find current state in NFA's transition map
            if (epsilon_transition != iterator->second.end()) { // If there are ε-transitions
                // Process each state reachable by ε-transitions
                for (int next : epsilon_transition->second) {
                    if (epsilon_closure.find(next) == epsilon_closure.end()) {
                        epsilon_closure.insert(next);
                        stack.push(next);
                    }
                }
            }
        }
    }
    return epsilon_closure;
}

//---------------------------------------------------------------------------------
// epsilon closure of a set of states in the NFA 
// a dfs traversal of epsilon transitions
// calls EpsilonClosureState on each state
//---------------------------------------------------------------------------------
set<int> NFA::EpsilonClosure(const set<int>& states) const {
    set<int> closure;
    for (int state : states) {
        set<int> single_closure = EpsilonClosureState(state);  // Call on each individual state
        closure.insert(single_closure.begin(), single_closure.end());
    }
    return closure;
}

//---------------------------------------------------------------------------------
// move operation for the NFA
// returns the set of states reachable from a set of states on a given symbol
//---------------------------------------------------------------------------------
set<int> NFA::move(const set<int> &T, char symbol) const
{
    set<int> result;
    for (int state : T)
    {
        // Check if there is any transition from state
        auto iterator = Ntran.find(state);
        if (iterator != Ntran.end())
        {
            // Check if a transition exists on the given symbol
            auto transIterator = iterator->second.find(symbol);
            if (transIterator != iterator->second.end())
            {
                // Insert all destination states into result
                result.insert(transIterator->second.begin(), transIterator->second.end());
            }
        }
    }
    return result;
}

//---------------------------------------------------------------------------------
// merge given NFA into NFA calling this function. 
// combines alphabets and merges transitions
//---------------------------------------------------------------------------------
void NFA::Merge(const NFA &nfaToMerge)
{
    // Copy each transition from other NFA
    for (auto &row : nfaToMerge.Ntran)
    {
        int src = row.first;
        for (auto &trans : row.second)
        {
            char sym = trans.first;
            // Add new destination states to any existing transitions
            Ntran[src][sym].insert(trans.second.begin(), trans.second.end());
        }
    }
    // Merge the alphabets
    alphabet.insert(nfaToMerge.alphabet.begin(), nfaToMerge.alphabet.end());
}

//---------------------------------------------------------------------------------
// shift all states by a given offset
//---------------------------------------------------------------------------------
void NFA::ShiftStates(int offset)
{
    map<int, map<char, set<int>>> newNtran;
    // Process each row in the current transition map
    for (const auto &row : Ntran)
    {
        int newSrc = row.first + offset; // Shift the current state by offset
        // Process each transition from this state
        for (const auto &trans : row.second)
        {
            char sym = trans.first;
            set<int> newDst;
            // Shift each state transition by offset
            for (int dest : trans.second)
            {
                newDst.insert(dest + offset);
            }
            newNtran[newSrc][sym] = newDst;
        }
    }
    Ntran = newNtran;
    init_state += offset;

    set<int> newFinStates;
    // Shift each final state by offset
    for (int s : fin_states)
    {
        newFinStates.insert(s + offset);
    }
    fin_states = newFinStates;
}