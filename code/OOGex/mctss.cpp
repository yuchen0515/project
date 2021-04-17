#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>
#include <utility>
#include <iomanip>
#include <string>
//#include <stdlib.h>
#include <time.h>
//#include <omp.h>
#include "stdafx.h"
#include "mcts.h"

//#define PRINT

//using namespace std;
//const int numThread = 10;

namespace MCTSS {
class Node {
 private:
    int32_t V, N;
    Node* const parent;
    const Move move;
    const std::vector<Move> moves;
    std::vector<Node*> children;

    Node* Select(State& state);
    Node* Expansion(State& state);
    int32_t Simulate(State& state) const;
    void Update(int32_t value);
    void OneRound(State state);

 public:
    Node(const State& state);
    Node(const State& state, Node* parent, Move move);
    ~Node();
    int32_t visit_num() const {
        return N;
    }
    double mean() const {
        return static_cast<double> (V) / N;
    }
    Move get_move() const {
        return move;
    }
    Move Round(const State& state, int32_t simLimit);
    string detail() const;
    void print(ostream& os) const;
};

Node::Node(const State& state)
    : V(0),
    N(0),
    parent(nullptr),
    move(State::no_move),
    moves(state.get_threat_moves()) {
    }

Node::Node(const State& state, Node* parent, Move move)
    : V(0),
    N(0),
    parent(parent),
    move(move),
    moves(state.get_threat_moves()) {
    }

Node::~Node() {
    for (auto child : children) {
        delete child;
    }
}

Node* Node::Select(State& state) {
    auto node = this;
    while (node->children.size() == node->moves.size()) {
        double best_value = -2.0;
        Node* best_child = nullptr;
        for (auto child : node->children) {
            double uct = static_cast<double> (child->V) / child->N + sqrt(2.0 * log(static_cast<double> (node->N)) / child->N);
            if (uct > best_value) {
                best_value = uct;
                best_child = child;
            }
        }
        if (best_child == nullptr) {
            break;
        }
        node = best_child;
        state.do_move(node->move);
    }
    return node;
}

Node* Node::Expansion(State& state) {
    auto node = this;
    auto it = children.size();
    auto move = moves[it];
    state.do_move(move);
    node = new Node(state, node, move);
    children.push_back(node);
    return node;
}

int32_t Node::Simulate(State& state) const {
    auto player_to_move = state.who_turns();
    //int sim_dep = 20;
    while (state.is_end() == false) {
        auto moves = state.get_threat_moves();
        auto move_num = moves.size();

        //srand(time(NULL));
        int32_t r = rand() % move_num;
        state.do_move(moves[r]);
        //sim_dep--;
        //if (sim_dep <= 0)
        //  return 0;
    }
    if (state.is_draw() == true) {
        return 0;
    }
    if (state.who_turns() == player_to_move) {
        return 1;
    }
    return -1;
    //return -2;
}

void Node::Update(int32_t value) {
    auto node = this;
    while (node != nullptr) {
        node->V += value;
        node->N += 1;
        value = -value;
        node = node->parent;
    }
}

void Node::OneRound(State state) {
    auto node = this;
    node = node->Select(state);
    if (state.is_end() == false) {
        node = node->Expansion(state);
    }
    int32_t result = 0;
    if (state.is_end() == true) {
        if (state.is_draw() == false) {
            result = 1;
        }
    } else {
        result += node->Simulate(state);
    }
    node->Update(result);
}

Move Node::Round(const State& state, int32_t simLimit) {
    //int sim_cnt = 0;
    /*//time limit
      __int64 rtime;

      rtime = clock() + simLimit *  CLOCKS_PER_SEC;
      while (clock() < rtime){
      OneRound(state);
    //sim_cnt++;
    }
    */
    //number limit
    while (simLimit--) {
        OneRound(state);
    }

    //std::cout << "sim_cnt=" << sim_cnt << endl;
    auto best_move = State::no_move;
    int32_t best_value = -1;
    for (auto child : children) {
        int32_t num = child->visit_num();
        if (num > best_value) {
            best_value = num;
            best_move = child->get_move();
        }
    }
    return best_move;
}

std::string Node::detail() const {
    double rate = mean() / 2 + 0.5;
    stringstream ss;
    ss << "Move:" << static_cast<char>(move % 15 + 65);
    ss << setw(2) << (15 - move / 15);
    ss << "\t Value:" << V;
    ss << "\t Visited:" << N;
    ss << "\t win-rate:" << rate;
    return ss.str();
}

void Node::print(ostream& os) const {
    for (auto child : children) {
        os << child->detail() << endl;
    }
}

ostream& operator<<(ostream& os, const Node& node) {
#ifdef PRINT
    node.print(os);
#endif
    return os;
}

Move MCTS(State state, int32_t simLimit) {
    Node root = Node(state);
    Move move = root.Round(state, simLimit);
#ifdef PRINT
    std::cout << root << endl;
#endif
    return move;
}

}  // namespace PURE
