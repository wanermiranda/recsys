//
// Created by gorigan on 10/6/15.
//

#ifndef TP1_RECSYS_DISTANCES_H
#define TP1_RECSYS_DISTANCES_H

#include <iostream>
#include <vector>

using namespace std;

auto distance_comparer = [](const std::pair<float, size_t> &a, const std::pair<float, size_t> &b) {
    return (a.first < b.first && a.second != b.second);
};


float cosine_distance(const float *vector1, const float *vector2, size_t size_cols);

float manhattan_distance(vector<float> vector1, vector<float> vector2);

#endif //TP1_RECSYS_DISTANCES_H
