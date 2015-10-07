//
// Created by gorigan on 10/6/15.
//

#include "Predictor.h"
#include <map>

std::mutex mtx;

void rank_for_vector(void *fv_pointer, size_t query_index, vector<vector<size_t>> &results, size_t size_rows,
                     size_t size_cols) {
    double distances[NN] = {2.0, 2.0, 2.0, 2.0, 2.0};
    size_t neighbors[NN];
    if (double **feature_vectors = reinterpret_cast<double **>(fv_pointer)) {
        for (size_t target_index = 0; target_index < size_rows; target_index++) {
            if (query_index != target_index) { // skipping itself
                double target_distance = cosine_distance(feature_vectors[query_index], feature_vectors[target_index],
                                                         size_cols);
                if (target_distance <= distances[NN - 1]) {
                    double hold_distance = target_distance;
                    size_t hold_neighbor = target_index;
                    // Rearrange neighbors
                    for (size_t counter = 0; counter < NN; counter++) {
                        if ((hold_distance <= distances[counter])) {

                            double aux_distance = distances[counter];
                            size_t aux_neighbor = neighbors[counter];

                            distances[counter] = hold_distance;
                            neighbors[counter] = hold_neighbor;

                            hold_distance = aux_distance;
                            hold_neighbor = aux_neighbor;
                        }
                    }

                }
            }
        }
    }


    mtx.lock();
    results.push_back(vector<size_t>(std::begin(neighbors), std::end(neighbors)));
    mtx.unlock();
}

std::string now(const char *format = "%c") {
    std::time_t t = std::time(0);
    char cstr[128];
    std::strftime(cstr, sizeof(cstr), format, std::localtime(&t));
    return cstr;
}


vector<vector<size_t> > rank_vectors(double **feature_vectors, size_t size_rows, size_t size_cols) {
    vector<vector<size_t> > results(size_rows, vector<size_t>(NN));
    void *first = (double *) feature_vectors;
    std::thread threads_active[NTHREADS];
    size_t threads_count = 0;
    for (size_t ind_ext = 0; ind_ext < size_rows; ind_ext++) {
        cout << "Computing distances for items: " << ind_ext + 1 << endl;
//        rank_for_vector(first, ind_ext, results, size_rows, size_cols);
        threads_active[threads_count] = std::thread(rank_for_vector, first, ind_ext, std::ref(results), size_rows,
                                                    size_cols);
        threads_count++;
        if (threads_count >= NTHREADS) {
            std::cout << now() << endl;
            threads_count = 0;
            for (size_t i = 0; i < NTHREADS; i++)
                threads_active[i].join();
        } else if (ind_ext + 1 == size_rows) {
            for (size_t i = 0; i < threads_count; i++)
                threads_active[i].join();
        }
    }
    return results;
}


void compute_stats_avg(vector<vector<double>> &stats) {// Overall average
    for (size_t index = 0; index < stats.size(); index++)
    if (stats[index][2] != -1){
        stats[index][1] = stats[index][2] / stats[index][0];
        // Zeroing the sum to a new average
        stats[index][2] = 0;
    }
}


void review_predictions(const vector<vector<string>> &targets, vector<double> &predictions,
                        vector<double> &missing_predictions) {
    // From the missing predictions, speculate a possible value based on the average ratings 4 same users or items
    for (size_t missing_index = 0; missing_index < missing_predictions.size(); missing_index++) {
        size_t target_pos = missing_predictions[missing_index];
        string user_id = targets[target_pos][0];
        string item_id = targets[target_pos][1];
        double item_count = 0, user_count = 0;
        double item_avg = 0, user_avg = 0;
        if (target_pos == targets.size() - 1)
            cout << "Last" << endl;

        for (size_t target_index = 0; target_index < targets.size(); target_index++)
            if ((targets[target_index][0] != user_id) || (targets[target_index][1] == item_id)) {
                if (targets[target_index][0] == user_id) {
                    user_avg += predictions[target_index];
                    user_count++;
                }
                if (targets[target_index][1] == item_id) {
                    item_avg += predictions[target_index];
                    item_count++;
                }
            }
        // filling the missing with an average rating from user or item
        if (user_count > 0) {
            user_avg /= user_count;
            predictions[target_pos] = user_avg;
        } else if (item_count > 0) {
            item_avg /= item_count;
            predictions[target_pos] = item_avg;
        }


    }
}


void avg_predictions(unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                     const vector<vector<double>> &users_stats, const vector<vector<double>> &items_stats,
                     const vector<vector<string>> &targets,
                     vector<double> &predictions, vector<double> &missing_predictions) {
    {
        for (int index = 0; index < targets.size(); index++) {

            int item_pos = items.at(targets[index][1]);
            int user_pos = users.at(targets[index][0]);

            if (items_stats[item_pos][2] != -1)
                predictions.push_back(items_stats[item_pos][1]);
            else if (users_stats[user_pos][2] != -1)
                predictions.push_back(users_stats[user_pos][1]);
            else {
                predictions.push_back(5);
                missing_predictions.push_back(index);
            }


        }
    }
}


void avg_predictions_personalized(unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                                  const vector<vector<double>> &users_stats, const vector<vector<double>> &items_stats,
                                  const vector<vector<string>> &targets,
                                  vector<double> &predictions, vector<double> &missing_predictions) {
    {

        for (int index = 0; index < targets.size(); index++) {
            double user_avg = 5;
            double item_avg = 5;
            // getting the user bias
            if (users.find(targets[index][0]) != users.end()) {
                size_t user_pos = users.at(targets[index][0]);
                user_avg = users_stats[user_pos][1];
            }

            if (items.find(targets[index][1]) != items.end()) {
                int item_pos = items.at(targets[index][1]);
                item_avg = items_stats[item_pos][1];
#ifdef BIAS
                predictions.push_back(user_avg + items_stats[item_pos][1]);
#else
                predictions.push_back(items_stats[item_pos][1]);
#endif

            }
            else if (users.find(targets[index][0]) != users.end()) {
                predictions.push_back(user_avg);
            }
            else {
                predictions.push_back(user_avg);
                missing_predictions.push_back(index);
            }

            if (predictions[index] > 10) {
                cout << "Error " << predictions[index] << "= " << item_avg << " + " << user_avg << endl;
                predictions[index] = 10;
            }
            if (predictions[index] < 0) {
                cout << "Error " << predictions[index] << "= " << item_avg << " + " << user_avg << endl;
                predictions[index] = 00;
            }


        }
    }
}


void user_predictions(unordered_map<string, size_t> &users, const vector<vector<double>> &users_stats,
                      const vector<vector<size_t>> &ranking_user, const vector<vector<string>> &targets,
                      vector<double> &predictions, vector<double> &missing_predictions) {
    for (size_t index = 0; index < targets.size(); index++) {
        if (users.find(targets[index][0]) != users.end()) {
            size_t target_user_pos = users.at(targets[index][0]);
            double avg = 0;

            // Capturing the average rating from the nearest neighbors
            for (size_t vect_index = 0; vect_index < NN; vect_index++) {
                size_t user_pos = ranking_user[target_user_pos][vect_index];
                avg += double(users_stats[user_pos][1]);
            }
            // Average them
            avg /= NN;
            predictions.push_back(avg);
        }
        else {
            predictions.push_back(5);
            missing_predictions.push_back(index);
        }

    }
}


void item_predictions(unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                      const vector<vector<double>> &users_stats, const vector<vector<double>> &items_stats,
                      const vector<vector<size_t>> &ranking_item, const vector<vector<string>> &targets,
                      vector<double> &predictions, vector<double> &missing_predictions) {
    for (size_t index = 0; index < targets.size(); index++) {
        double user_avg = 5;
        // getting the user bias
        if (users.find(targets[index][0]) != users.end()) {
            size_t user_pos = users.at(targets[index][0]);
            user_avg = users_stats[user_pos][1];
        }

        // Look for the item within the ranking
        if (items.find(targets[index][1]) != items.end()) {
            size_t target_item_pos = items.at(targets[index][1]);
            double item_avg = 0;

            // Capturing the average rating from the nearest neighbors of the item
            for (size_t vect_index = 0; vect_index < NN; vect_index++) {
                size_t item_pos = ranking_item[target_item_pos][vect_index];
                item_avg += double(items_stats[item_pos][1]);
            }
            // Average them
            item_avg /= NN;
            // adding user bias or if there is no user, adds  5.
            //predictions.push_back(user_avg + item_avg);
            predictions.push_back(item_avg);
        }// in case there is no item, set only the user bias average or if there is no user, set to 5.
        else {
            predictions.push_back(user_avg);
            if (users.find(targets[index][0]) == users.end())
                missing_predictions.push_back(index);
        }
        if (index == targets.size() - 1)
            cout << "Last" << endl;
    }
}
