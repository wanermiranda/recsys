//
// Created by gorigan on 10/6/15.
//

#include <cmath>
#include "distances.h"


float cosine_distance(const float *vector1, const float *vector2,size_t size_cols) {
    float dot_product = 0.0, sum_v1 = 0.0, sum_v2 = 0.0;
    float norm_v1, norm_v2;


    for (size_t index = 0; index < size_cols; index ++) {

        if ((vector1[index] + vector2[index] != 0)) {
            dot_product += vector1[index] * vector2[index];
            sum_v1 += vector1[index] * vector1[index];
            sum_v2 += vector2[index] * vector2[index];
        }
    }

    norm_v1 = sqrt(sum_v1);
    norm_v2 = sqrt(sum_v2);

    return dot_product / (norm_v1 * norm_v2);
}


float manhattan_distance(vector<float> vector1, vector<float> vector2) {
    float dist = 0;

    for (size_t index = 0; index < vector1.size(); index++) {
        dist += abs(vector1[index] - vector2[index]);
    }
    return dist;
}