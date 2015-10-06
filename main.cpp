#include <iostream>
#include <map>
#include "StringUtils.h"
#include "CSVReader.h"
#include <unordered_map>
#include <algorithm>
#include <set>
#include <thread>
#include <mutex>

using namespace std;

#define NN 7
#define NTHREADS 7

std::mutex mtx;

double **alloc_double_fv(size_t size_rows, size_t size_cols) {
    double **a = (double **) malloc(sizeof *a * size_rows);
    if (a)
    {
        for (int i = 0; i < size_rows; i++)
        {
            a[i] = (double *) malloc(sizeof *a[i] * size_cols);
        }
    }
    return a;
}


auto distance_comparer = [](const std::pair<double, size_t>& a, const std::pair<double, size_t>& b) {
    return (a.first < b.first && a.second != b.second);
};

void rank_vector(const vector<vector<double>> &feature_vectors, size_t query_index, vector< vector<size_t>> &results);

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


double cosine_distance(const double* vector1, const double* vector2, size_t size_cols) {
    double dot_product = 0.0, sum_v1 = 0.0, sum_v2 = 0.0;
    double norm_v1, norm_v2;

    long size = size_cols;

    for (size_t index = 0; index < size; index ++){
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

    for (size_t index = 0; index < vector1.size(); index ++){
        dist += abs(vector1[index] - vector2[index]);
    }
    return dist;
}

void rank_for_vector(void *fv_pointer, size_t query_index, vector<vector<size_t>> &results, size_t size_rows,
                     size_t size_cols)  {
    double distances[NN] = {2.0,2.0,2.0,2.0,2.0};
    size_t neighbors[NN];
    if (double** feature_vectors = reinterpret_cast<double**>(fv_pointer)) {
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
std::string now( const char* format = "%c" )
{
    std::time_t t = std::time(0) ;
    char cstr[128] ;
    std::strftime( cstr, sizeof(cstr), format, std::localtime(&t) ) ;
    return cstr ;
}

vector< vector<size_t> > rank_vectors(double **feature_vectors, size_t size_rows, size_t size_cols){
    vector< vector<size_t> > results(size_rows, vector<size_t>(NN));
    void *first = (double*)feature_vectors;
    std::thread threads_active[NTHREADS];
    size_t threads_count = 0;
    for (size_t ind_ext = 0; ind_ext < size_rows; ind_ext++) {
        cout << "Computing distances for items: " << ind_ext + 1 << endl;
//        rank_for_vector(first, ind_ext, results, size_rows, size_cols);
        threads_active[threads_count] = std::thread(rank_for_vector, first, ind_ext, std::ref(results), size_rows,
                                                    size_cols);
        threads_count ++;
        if (threads_count >= NTHREADS) {
            std::cout << now() << endl;
            threads_count = 0;
            for (size_t i = 0; i < NTHREADS; i++)
                threads_active[i].join();
        } else if (ind_ext+1 == size_rows) {
            for (size_t i = 0; i < threads_count; i++)
                threads_active[i].join();
        }
    }
    return results;
}


int main(int argc,  char** argv) {
    if (argc < 4) {
        cout << "usage: ./TP1_Recsys ratings.csv targets.csv output.csv";
        exit(1);
    }
    ifstream ratings_file(argv[1]);
    unordered_map<string,size_t> users;
    unordered_map<string,size_t> items;

    // The stats will hold the values for {count, avg, sum},
    vector<vector<double>> users_stats;
    vector<vector<double>> items_stats;

    vector< vector<string> > rows;
    size_t user_count = 0, item_count = 0, row_count = 0;

    CSVReader row_reader;
    // Skip the header
    ratings_file >> row_reader;
    while(ratings_file >> row_reader)
    {
        vector<string> user_item = split(row_reader[0], ':');
        rows.push_back(vector<string>({user_item[0], user_item[1], row_reader[1]}));
        row_count++;
        double vote = stod(row_reader[1]);

        if (users.find(user_item[0]) == users.end()) {
            users.insert({user_item[0], user_count++});
            users_stats.push_back(vector<double>({1,0,vote}));
        }
        else {
            size_t user_pos = users.at(user_item[0]);
            users_stats[user_pos][0]++;
            users_stats[user_pos][2] = users_stats[user_pos][2] + vote;
        }


        if (items.find(user_item[1]) == items.end()) {
            items.insert({user_item[1], item_count++});
            items_stats.push_back(vector<double>({1, 0, 0}));
        }
        else {
            size_t item_pos = items.at(user_item[1]);
            items_stats[item_pos][0] ++;
        }

    }
    ratings_file.close();

    // Computing users average
    compute_stats_avg(users_stats);


    cout << "Rows: " << rows.size() << " Users: " << users.size() << " Items: " << items.size() << endl;
    cout << "Transposing  data..." << endl;
    double **items_fvs = alloc_double_fv(item_count, user_count);
    for (size_t index = 0; index < rows.size(); index++){
        size_t user_pos = users.at(rows[index][0]);
        size_t item_pos = items.at(rows[index][1]);
        double vote = stod(rows[index][2]); //- users_stats[user_pos][1];

        users_stats[user_pos][2] += vote;
        items_stats[item_pos][2] += vote;

        items_fvs[item_pos][user_pos] = vote;
    }

    // Computing item average after removing the user bias
    compute_stats_avg(items_stats);


    cout << "Computing and Ranking Similiraties..." << endl;
    rows.clear();

    vector< vector<size_t> > ranking_item = rank_vectors(items_fvs, item_count, user_count);

    delete items_fvs;

    cout << "Reading targets..." << endl;
    ifstream targets_file(argv[2]);
    vector< vector<string> > targets;

    // Skip the header
    targets_file >> row_reader;
    while(targets_file >> row_reader)
        targets.push_back(split(row_reader[0],':'));

    targets_file.close();

    cout << "Pre-processing predictions..." << endl;
    vector<double> predictions;
    vector<double> missing_predictions;
    item_predictions(users, items, users_stats, items_stats, ranking_item, targets, predictions, missing_predictions);


    users.clear();
    items.clear();
    users_stats.clear();

    cout << "Review predictions..." << endl;
    review_predictions(targets, predictions, missing_predictions);


    cout << "Doing predictions..." << endl;
    ofstream mixed_output(argv[3]);
    mixed_output << "UserId:ItemId,Prediction" << endl;
    for (size_t index = 0; index < targets.size(); index++){
            mixed_output << targets[index][0] << ':' << targets[index][1] << ','
            << predictions[index] << endl;
    }

    mixed_output.close();
    return 0;
}

void compute_stats_avg(vector<vector<double>> &stats) {// Overall average
    for (size_t index = 0; index < stats.size(); index ++) {
        stats[index][1] = stats[index][2] / stats[index][0];
        // Zeroing the sum to a new average
        stats[index][2] = 0;
    }
}


void review_predictions(const vector<vector<string>> &targets, vector<double> &predictions,
                                  vector<double> &missing_predictions) {
    // From the missing predictions, speculate a possible value based on the average ratings 4 same users or items
    for (size_t missing_index = 0; missing_index < missing_predictions.size(); missing_index++){
        size_t target_pos = missing_predictions[missing_index];
        string user_id = targets[target_pos][0];
        string item_id = targets[target_pos][1];
        double item_count = 0, user_count = 0;
        double item_avg = 0, user_avg = 0;
        if (target_pos == targets.size() - 1)
            cout << "Last" << endl;

        for (size_t target_index = 0; target_index < targets.size(); target_index ++)
            if (target_index != missing_index){
                if (targets[target_index][0] == user_id)  {
                    user_avg += predictions[target_index];
                    user_count ++;
                }
                if (targets[target_index][1] == item_id) {
                    item_avg += predictions[target_index];
                    item_count ++;
                }
        }
        // filling the missing with an average rating from user or item
        if (item_count > 0) {
            item_avg /= item_count;
            predictions[target_pos] = item_avg;
        }
        else
        if (user_count > 0) {
            user_avg /= user_count;
            predictions[target_pos] = user_avg;
        }


    }
}

void user_predictions(unordered_map<string, size_t> &users, const vector<vector<double>> &users_stats,
                      const vector<vector<size_t>> &ranking_user, const vector<vector<string>> &targets,
                      vector<double> &predictions, vector<double> &missing_predictions) {
    for (size_t index = 0; index < targets.size(); index++){
        if (users.find(targets[index][0]) != users.end()) {
            size_t target_user_pos = users.at(targets[index][0]);
            double avg = 0;

            // Capturing the average rating from the nearest neighbors
            for (size_t vect_index = 0; vect_index < NN; vect_index ++) {
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
                      vector<double> &predictions, vector<double> &missing_predictions){
    for (size_t index = 0; index < targets.size(); index++){
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
            for (size_t vect_index = 0; vect_index < NN; vect_index ++) {
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
