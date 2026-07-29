#include "mate_solver.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

#include "../core/movegen.h"

namespace pioneer {

namespace {

using ProofNumber = uint32_t;
constexpr ProofNumber PN_INFINITY = 1u << 30;

ProofNumber saturated_add(ProofNumber a, ProofNumber b) {
    if (a >= PN_INFINITY || b >= PN_INFINITY - a) return PN_INFINITY;
    return a + b;
}

struct Node;

struct Child {
    Move move;
    std::unique_ptr<Node> node;
};

struct Node {
    ProofNumber proof = 1;
    ProofNumber disproof = 1;
    bool expanded = false;
    std::vector<Child> children;
};

class Solver {
   public:
    Solver(const Position& root, int max_plies, uint64_t node_budget)
        : attacker_(root.side_to_move()),
          max_plies_(std::max(0, max_plies)),
          node_budget_(node_budget) {}

    MateResult solve(const Position& pos) {
        MateResult result;
        if (node_budget_ == 0) {
            result.nodes = 0;
            return result;
        }

        Node root;
        nodes_ = 1;
        path_.push_back(pos.key());
        dfpn(root, pos, 0, PN_INFINITY, PN_INFINITY);

        result.proven = root.proof == 0;
        result.complete = result.proven || root.disproof == 0;
        result.nodes = nodes_;
        result.proof = root.proof;
        result.disproof = root.disproof;

        if (result.proven) {
            for (const Child& child : root.children) {
                if (child.node->proof == 0) {
                    result.first_move = child.move;
                    break;
                }
            }
        }
        return result;
    }

   private:
    void set_proven(Node& node) {
        node.expanded = true;
        node.proof = 0;
        node.disproof = PN_INFINITY;
    }

    void set_disproven(Node& node) {
        node.expanded = true;
        node.proof = PN_INFINITY;
        node.disproof = 0;
    }

    void update(Node& node, bool attacker_turn) {
        assert(node.expanded && !node.children.empty());
        if (attacker_turn) {
            node.proof = PN_INFINITY;
            node.disproof = 0;
            for (const Child& child : node.children) {
                node.proof = std::min(node.proof, child.node->proof);
                node.disproof =
                    saturated_add(node.disproof, child.node->disproof);
            }
        } else {
            node.proof = 0;
            node.disproof = PN_INFINITY;
            for (const Child& child : node.children) {
                node.proof = saturated_add(node.proof, child.node->proof);
                node.disproof =
                    std::min(node.disproof, child.node->disproof);
            }
        }
    }

    bool expand(Node& node, const Position& pos, int ply) {
        const bool attacker_turn = pos.side_to_move() == attacker_;
        MoveList legal;
        generate_legal(pos, legal);

        std::vector<Move> relevant;
        if (attacker_turn) {
            if (ply >= max_plies_) {
                set_disproven(node);
                return true;
            }
            for (Move move : legal)
                if (gives_check(pos, move)) relevant.push_back(move);
            if (relevant.empty()) {
                set_disproven(node);
                return true;
            }
        } else {
            // Every attacker child is a checking move, so reaching a
            // defender node outside check cannot prove this tsume.
            if (!pos.in_check()) {
                set_disproven(node);
                return true;
            }
            if (legal.empty()) {
                // generate_legal already removes illegal pawn-drop mate.
                // Therefore a pawn drop can never create this proof leaf.
                set_proven(node);
                return true;
            }
            // Checkmate at the horizon is recognized above; any legal
            // evasion at or beyond it refutes the bounded proposition.
            if (ply >= max_plies_) {
                set_disproven(node);
                return true;
            }
            relevant.assign(legal.begin(), legal.end());
        }

        const uint64_t needed = static_cast<uint64_t>(relevant.size());
        if (needed > node_budget_ - nodes_) {
            budget_exhausted_ = true;
            return false;
        }

        node.children.reserve(relevant.size());
        for (Move move : relevant)
            node.children.push_back({move, std::make_unique<Node>()});
        nodes_ += needed;
        node.expanded = true;
        update(node, attacker_turn);
        return true;
    }

    size_t most_proving_child(const Node& node, bool attacker_turn,
                              ProofNumber& second_best) const {
        size_t best_index = 0;
        ProofNumber best = PN_INFINITY;
        second_best = PN_INFINITY;

        for (size_t i = 0; i < node.children.size(); ++i) {
            const Node& child = *node.children[i].node;
            const ProofNumber value =
                attacker_turn ? child.proof : child.disproof;
            if (value < best) {
                second_best = best;
                best = value;
                best_index = i;
            } else if (value < second_best) {
                second_best = value;
            }
        }
        return best_index;
    }

    void search_child(Child& child, const Position& pos, int ply,
                      ProofNumber proof_threshold,
                      ProofNumber disproof_threshold) {
        Position next = pos;
        next.apply(child.move);

        if (std::find(path_.begin(), path_.end(), next.key()) != path_.end()) {
            // Position lacks pre-root history, so v1 rejects even the
            // first within-search recurrence. This can miss a mate but
            // cannot turn repetition into a checkmate proof.
            set_disproven(*child.node);
            return;
        }

        path_.push_back(next.key());
        dfpn(*child.node, next, ply + 1, proof_threshold,
             disproof_threshold);
        path_.pop_back();
    }

    void dfpn(Node& node, const Position& pos, int ply,
              ProofNumber proof_threshold,
              ProofNumber disproof_threshold) {
        if (budget_exhausted_) return;
        if (!node.expanded && !expand(node, pos, ply)) return;
        if (node.children.empty()) return;  // terminal proof/disproof

        const bool attacker_turn = pos.side_to_move() == attacker_;
        while (node.proof < proof_threshold &&
               node.disproof < disproof_threshold &&
               !budget_exhausted_) {
            ProofNumber second_best = PN_INFINITY;
            const size_t index =
                most_proving_child(node, attacker_turn, second_best);
            Child& child = node.children[index];

            ProofNumber child_proof_threshold;
            ProofNumber child_disproof_threshold;
            if (attacker_turn) {
                child_proof_threshold =
                    std::min(proof_threshold,
                             saturated_add(second_best, 1));
                assert(disproof_threshold == PN_INFINITY ||
                       disproof_threshold > node.disproof);
                child_disproof_threshold =
                    disproof_threshold == PN_INFINITY
                        ? PN_INFINITY
                        : saturated_add(disproof_threshold - node.disproof,
                                        child.node->disproof);
            } else {
                assert(proof_threshold == PN_INFINITY ||
                       proof_threshold > node.proof);
                child_proof_threshold =
                    proof_threshold == PN_INFINITY
                        ? PN_INFINITY
                        : saturated_add(proof_threshold - node.proof,
                                        child.node->proof);
                child_disproof_threshold =
                    std::min(disproof_threshold,
                             saturated_add(second_best, 1));
            }

            search_child(child, pos, ply, child_proof_threshold,
                         child_disproof_threshold);
            update(node, attacker_turn);
        }
    }

    Color attacker_;
    int max_plies_;
    uint64_t node_budget_;
    uint64_t nodes_ = 0;
    bool budget_exhausted_ = false;
    std::vector<uint64_t> path_;
};

}  // namespace

MateResult solve_mate(const Position& pos, int max_plies,
                      uint64_t node_budget) {
    return Solver(pos, max_plies, node_budget).solve(pos);
}

}  // namespace pioneer
