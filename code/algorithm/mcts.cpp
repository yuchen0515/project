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
#include <limits>
#include <string>
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
            double V;
            int32_t N;

            Node* const parent;
            const Move move;
            const std::vector<Move> moves;
            std::vector<Node*> children;

            Node* Select(State& state);
            Node* Expansion(State& state);
            double Simulate(State& state) const;
            void Update(double value);
            void OneRound(State state);
            bool Expanded() const;
    };

    // Build Constructor
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

    // 解構子
    Node::~Node() {
        for (auto child : children) {
            delete child;
        }
    }

    // STEP 1: select
    Node* Node::Select(State& state) {
        auto node = this;
        //while (node->children.size() == node->moves.size()) {
        while (node->Expanded()) {
            //test
            //
            //std::cout << "turns: " << state.get_turns() << std::endl;
            //
            //double best_value = -2.0;
            double best_value = -std::numeric_limits<double>::max();
            Node* best_child = nullptr;

            // choose the best move to expand
            for (auto child : node->children) {
                double uct =
                    static_cast<double> (child->V) / (child->N + 1) \
                    + sqrt(2.0 * log(static_cast<double> (node->N)) / (child->N + 1));
                    //static_cast<double> (child->V) / child->N \
                    //+ sqrt(2.0 * log(static_cast<double> (node->N)) / child->N);

                //std::cout << "uct: " << uct << std::endl;
                if (uct > best_value) {
                    best_value = uct;
                    best_child = child;
                }
            }
            assert(best_child);
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

    // STEP 2: expansion
    Node* Node::Expansion(State& state) {
        std::mt19937_64 rng(std::chrono::system_clock::now().time_since_epoch().count());
        auto node = this;
        //auto it = children.size();
        //auto it = rng() % moves.size();
        //std::cout << "hihiihihihihihihi" << std::endl;
        //std::cout << it << std::endl;
        //std::cout << moves.size() << std::endl;
        
        //
        //auto move = moves[it];
        //state.do_Move(move);
        //node = new Node(state, node, move);
        //children.emplace_back(node);
        
        for (size_t i = 0 ; i < moves.size(); ++i) {
            children.emplace_back(new Node(state, this, moves[i]));
        }

        return node;
    }

    // STEP 3: simulate
    double Node::Simulate(State& state) const {
        auto player_to_move = state.get_turns();
        int32_t sim_dep = 7;

        // if (player_to_move == 1) { // Lower
        //     sim_dep = 7;
        // }
        
        int32_t best_move = 0;

        std::mt19937_64 rng(std::chrono::system_clock::now().time_since_epoch().count());

        // 模擬 sim_dep 次 或到 棋局結束
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
            if (int32_t myself = (state.get_turns() == 0 ? 0 : 1);
                    sim_dep <= 0){
                //std::cerr << "myself " << myself << ": " << state.sigmoid(state.get_board_score(myself) - state.get_board_score(1 - myself)) << std::endl;
                return state.sigmoid(state.get_board_score(myself) - state.get_board_score(1 - myself));
            }
            //std::cout << "Hello kity" << std::endl;
            //std::cout << move_num << std::endl;
            //std::cout << r << std::endl;
            ////////
            //
            //std::cout << "Player: " << state.player_to_move_ << std::endl;
        }

    // 平手判斷 (5五將棋原則上不影響)
    if (state.is_Draw() == true) {
        return 0;
    }

    // 勝利
    if (state.get_turns() == player_to_move) {
        //return 30000;
        return 1;
    }
    //return -30000;
    // 輸掉
    return -1;
    //return -2;
    }

    // STEP 4: update
    void Node::Update(double value) {
        auto node = this;
        while (node != nullptr) {
            node->V += value;
            //node->V += node->move.value;
            node->N += 1;
            value = -value;
            node = node->parent;
        }
    }

    // 判斷是否擴展過
    bool Node::Expanded() const {
        return !(children.empty());
    }

    // 跑過一輪 STEP 1 ~ 4
    void Node::OneRound(State state) {
        auto node = this;
        node = node->Select(state);

        //if (state.is_End() == false) {
        if (state.is_End() == false && node->Expanded() == false) {
            node = node->Expansion(state);
        }
        double result = 0.0;
        if (state.is_End() == true) {
            //std::cout << "ya" << std::endl;
            //if (state.is_Draw() == false) {
            result = 1.0;
            //result = 30000;
            //}
        } else {
            result = node->Simulate(state);
        }

        //node->Update(result);
        //node->Update((result + state.sigmoid(node->move.value) * 2) / 3);
        node->Update((2 * (result - 0.5) + state.sigmoid(node->move.value) * 2) / 3);
        //node->Update(node->move.value + result * 3000);
    }

    // 模擬 N 輪
    Move Node::Round(const State& state, int32_t simLimit) {
        //number limit
        while (simLimit--) {
            OneRound(state);
        }

        auto best_move = State::no_move;

        int32_t best_value = -std::numeric_limits<int32_t>::max();
        //int32_t best_value = -1;
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

    // 印出詳情
    std::string Node::detail() const {
        double rate = mean() / 2 + 0.5;
        std::stringstream ss;
        ss << "Move from: ( " << move.from.first << ", " << move.from.second << " )" << " | ";
        ss << "Move to: ( " << move.to.first << ", " << move.to.second << " )" << " | ";

        ss << "\t Value:" << V;
        ss << "\t Visited:" << N;
        ss << "\t ch:" << children.size();
        ss << "\t win-rate:" << rate;
        return ss.str();
    }

    // 印出所有 children
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
