//
// Created by gorigan on 10/6/15.
//

#ifndef TP1_RECSYS_PREDICTOR_H
#define TP1_RECSYS_PREDICTOR_H
#define OVERFLOW_DISTANCE -1
#define NN 1000
#define NTHREADS 7
//#define DEBUG
#define BIAS

#include <iostream>
#include <thread>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <thread>
#include <mutex>
#include <sys/time.h>

#include "distances.h"

using namespace std;


typedef unsigned long long timestamp_t;

static timestamp_t
get_timestamp ()
{
    struct timeval now;
    gettimeofday (&now, NULL);
    return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}


void avg_predictions_personalized(unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                                  const vector<vector<float>> &users_stats, const vector<vector<float>> &items_stats,
                                  const vector<vector<string>> &targets,
                                  vector<float> &predictions, vector<float> &missing_predictions);

void avg_predictions(unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                     const vector<vector<float>> &users_stats, const vector<vector<float>> &items_stats,
                     const vector<vector<string>> &targets,
                     vector<float> &predictions, vector<float> &missing_predictions);

vector<vector<pair<size_t, float>>> rank_vectors(float **feature_vectors, vector<size_t> targets,
                                                 size_t size_rows,size_t size_cols);

void user_predictions(unordered_map<string, size_t> &items, unordered_map<string, size_t> &users,
                      vector<vector<float>> &items_stats, vector<vector<float>> &users_stats,
                      vector<vector<pair<size_t, float>>> &ranking_user, vector<vector<string>> &targets,
                      vector<size_t> target_users,
                      vector<float> &predictions, vector<float> &missing_predictions, float **users_fvs
                    );

void item_predictions(unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                      vector<vector<float>> &users_stats, vector<vector<float>> &items_stats,
                      vector<vector<pair<size_t, float>>> &ranking_item, vector<vector<string>> &targets,
                      vector<float> &predictions, vector<float> &missing_predictions, float **items_fvs);

void review_predictions(const vector<vector<string>> &targets, vector<float> &predictions,
                        vector<float> &missing_predictions);

void compute_stats_avg(vector<vector<float>> &stats);


#endif //TP1_RECSYS_PREDICTOR_H
