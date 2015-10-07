//
// Created by gorigan on 10/6/15.
//

#include <cmath>
#include "distances.h"


double cosine_distance(const double *vector1, const double *vector2, size_t size_cols) {
    double dot_product = 0.0, sum_v1 = 0.0, sum_v2 = 0.0;
    double norm_v1, norm_v2;

    long size = size_cols;

    for (size_t index = 0; index < size; index++) {
        dot_product += vector1[index] * vector2[index];
        sum_v1 += vector1[index] * vector1[index];
        sum_v2 += vector2[index] * vector2[index];
    }

    norm_v1 = sqrt(sum_v1);
    norm_v2 = sqrt(sum_v2);

    double simmilarity = dot_product / (norm_v1 * norm_v2);
    return 1 - simmilarity;
}


double manhattan_distance(vector<double> vector1, vector<double> vector2) {
    double dist = 0;

    for (size_t index = 0; index < vector1.size(); index++) {
        dist += abs(vector1[index] - vector2[index]);
    }
    return dist;
}