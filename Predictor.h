//
// Created by gorigan on 10/6/15.
//

#ifndef TP1_RECSYS_PREDICTOR_H
#define TP1_RECSYS_PREDICTOR_H

#define NN 7
#define NTHREADS 8

#include <iostream>
#include <thread>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <thread>
#include <mutex>

#include "distances.h"

using namespace std;


void avg_predictions_personalized(unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                                  const vector<vector<double>> &users_stats, const vector<vector<double>> &items_stats,
                                  const vector<vector<string>> &targets,
                                  vector<double> &predictions, vector<double> &missing_predictions);

void avg_predictions(unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                     const vector<vector<double>> &users_stats, const vector<vector<double>> &items_stats,
                     const vector<vector<string>> &targets,
                     vector<double> &predictions, vector<double> &missing_predictions);

vector<vector<size_t> > rank_vectors(double **feature_vectors, size_t size_rows, size_t size_cols);

void user_predictions(unordered_map<string, size_t> &users, const vector<vector<double>> &users_stats,
                      const vector<vector<size_t>> &ranking_user, const vector<vector<string>> &targets,
                      vector<double> &predictions, vector<double> &missing_predictions);

void item_predictions(unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                      const vector<vector<double>> &users_stats, const vector<vector<double>> &items_stats,
                      const vector<vector<size_t>> &ranking_item, const vector<vector<string>> &targets,
                      vector<double> &predictions, vector<double> &missing_predictions);

void review_predictions(const vector<vector<string>> &targets, vector<double> &predictions,
                        vector<double> &missing_predictions);

void compute_stats_avg(vector<vector<double>> &stats);


#endif //TP1_RECSYS_PREDICTOR_H
