//
// Created by gorigan on 10/6/15.
//

#ifndef TP1_RECSYS_DISTANCES_H
#define TP1_RECSYS_DISTANCES_H

#include <iostream>
#include <vector>

using namespace std;

auto distance_comparer = [](const std::pair<double, size_t> &a, const std::pair<double, size_t> &b) {
    return (a.first < b.first && a.second != b.second);
};


double cosine_distance(const double *vector1, const double *vector2, size_t size_cols);

double manhattan_distance(vector<double> vector1, vector<double> vector2);

#endif //TP1_RECSYS_DISTANCES_H
