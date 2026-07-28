// Turning a match score into a rating difference, with the uncertainty
// attached.
//
// A bare "won 62 of 100" says little on its own: the interval around it
// is what decides whether a change was an improvement or noise. Phase 4
// replaces this pairwise estimate with a whole-pool fit, but the same
// logistic curve underlies both.

#pragma once

namespace pioneer {

struct EloEstimate {
    double elo = 0.0;
    double lower = 0.0;
    double upper = 0.0;
    // False when the score is 0 or 1: the estimate is then unbounded on
    // one side and only the finite bound means anything.
    bool finite = true;
};

// score is A's share of decisive games, in [0, 1].
double elo_difference(double score);

// Wilson interval on the score, mapped through the logistic curve.
// Wilson rather than the textbook normal approximation because scores
// near 0 and 1 are exactly where a new agent lands.
EloEstimate elo_estimate(double score, int decisive_games,
                         double z = 1.959964);

}  // namespace pioneer
