#include <time.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>
#include <utility>
#include <iomanip>
#include <string>
//#include <stdlib.h>
//#include <omp.h>
#include "mcts.h"

#define PRINT

//using namespace std;
//const int numThread = 10;

namespace PURE {
    class Node {
        public:
            Node(const State& state);
            Node(const State& state, Node* parent, Move move);
            ~Node();
            int32_t visit_num() const {
                return N;
            }
            double mean() const {
                return (static_cast<double> (V)) / N;
            }
            Move get_move() const {
                return move;
            }
            Move Round(const State& state, int32_t simLimit);
            std::string detail() const;
            void print(std::ostream& os) const;

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
    };

    Node::Node(const State& state)
        : V(0),
        N(0),
        parent(nullptr),
        move(State::no_move),
        moves(state.get_Moves()) {
        }

    Node::Node(const State& state, Node* parent, Move move)
        : V(0),
        N(0),
        parent(parent),
        move(move),
        moves(state.get_Moves()) {
        }

    Node::~Node() {
        for (auto child : children) {
            delete child;
        }
    }

    Node* Node::Select(State& state) {
        auto node = this;
        while (node->children.size() == node->moves.size()) {
            //test
            //
            //std::cout << "turns: " << state.get_turns() << std::endl;
            //
            double best_value = -2.0;
            Node* best_child = nullptr;
            for (auto child : node->children) {
                double uct =
                    static_cast<double> (child->V) / child->N \
                    + sqrt(2.0 * log(static_cast<double> (node->N)) / child->N);
                    //static_cast<double> (child->V) / child->N \
                    //+ sqrt(2.0 * log(static_cast<double> (node->N)) / child->N);

                //std::cout << "uct: " << uct << std::endl;
                if (uct > best_value) {
                    best_value = uct;
                    best_child = child;
                }
            }
            if (best_child == nullptr) {
                break;
            }
            node = best_child;
            state.do_Move(node->move);
            //
            //state.switch_turn();
            //
            //node->moves = node->get_Move();
        }
        return node;
    }

    Node* Node::Expansion(State& state) {
        std::mt19937_64 rng(std::chrono::system_clock::now().time_since_epoch().count());
        auto node = this;
        auto it = children.size();
        //auto it = rng() % moves.size();
        //std::cout << "hihiihihihihihihi" << std::endl;
        //std::cout << it << std::endl;
        //std::cout << moves.size() << std::endl;
        
        auto move = moves[it];
        state.do_Move(move);
        //
        //state.switch_turn();
        //
        node = new Node(state, node, move);
        children.emplace_back(node);

        return node;
    }

    int32_t Node::Simulate(State& state) const {
        auto player_to_move = state.get_turns();
        int32_t sim_dep = 50;
        int32_t best_move = 0;

        std::mt19937_64 rng(std::chrono::system_clock::now().time_since_epoch().count());

        while (state.is_End() == false) {
            auto moves = state.get_Moves();
            auto move_num = moves.size();

            //srand(time(NULL));
            int32_t r = rng() % move_num;

            state.do_Move(moves[r]);
            //
            //state.switch_turn();
            //
            sim_dep--;
            //}
            if (sim_dep <= 0){
                return 0;
            }
            //std::cout << "Hello kity" << std::endl;
            //std::cout << move_num << std::endl;
            //std::cout << r << std::endl;
    }
    if (state.is_Draw() == true) {
        return 0;
    }
    if (state.get_turns() == player_to_move) {
        //return 30000;
        return 1;
    }
    //return -30000;
    return -1;
    //return -2;
}

void Node::Update(int32_t value) {
    auto node = this;
    while (node != nullptr) {
        node->V += value;
        //node->V += node->move.value;
        node->N += 1;
        value = -value;
        node = node->parent;
    }
}

void Node::OneRound(State state) {
    auto node = this;
    node = node->Select(state);

    if (state.is_End() == false) {
        node = node->Expansion(state);
    }
    int32_t result = 0;
    if (state.is_End() == true) {
        //std::cout << "ya" << std::endl;
        //if (state.is_Draw() == false) {
        result = 1;
        //result = 30000;
        //}
    } else {
        result = node->Simulate(state);
    }

    node->Update(result);
    //node->Update(node->move.value);
    //node->Update(node->move.value + result * 3000);
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
    //int32_t best_value = -30000;
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
    std::stringstream ss;
    ss << "Move from: ( " << move.from.first << ", " << move.from.second << " | ";
    ss << "Move to: ( " << move.to.first << ", " << move.to.second << " | ";

    //ss << "Move:" << static_cast<char>(move % 15 + 65);
    //ss << getw(2) << (15 - move / 15);
    ss << "\t Value:" << V;
    ss << "\t Visited:" << N;
    ss << "\t win-rate:" << rate;
    return ss.str();
}

void Node::print(std::ostream& os) const {
    for (auto child : children) {
        os << child->detail() << std::endl;
    }
}

std::ostream& operator<<(std::ostream& os, const Node& node) {
#ifdef PRINT
    node.print(os);
#endif
    return os;
}

Move MCTS(State state, int32_t simLimit) {
    Node root = Node(state);
    Move move = root.Round(state, simLimit);
#ifdef PRINT
    std::cout << root << std::endl;
#endif
    return move;
}

}  // namespace PURE
