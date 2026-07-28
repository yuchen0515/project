#include "rating.h"

#include <cmath>
#include <limits>

namespace pioneer {

double elo_difference(double score) {
    if (score <= 0.0) return -std::numeric_limits<double>::infinity();
    if (score >= 1.0) return std::numeric_limits<double>::infinity();
    return -400.0 * std::log10(1.0 / score - 1.0);
}

EloEstimate elo_estimate(double score, int decisive_games, double z) {
    EloEstimate out;
    if (decisive_games <= 0) return out;

    const double n = decisive_games;
    const double z2 = z * z;

    // Wilson score interval.
    const double denominator = 1.0 + z2 / n;
    const double centre = (score + z2 / (2.0 * n)) / denominator;
    const double spread =
        z * std::sqrt(score * (1.0 - score) / n + z2 / (4.0 * n * n)) /
        denominator;

    out.elo = elo_difference(score);
    out.lower = elo_difference(centre - spread);
    out.upper = elo_difference(centre + spread);
    out.finite = std::isfinite(out.elo);
    return out;
}

}  // namespace pioneer
