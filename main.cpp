#include <iostream>
#include <map>
#include <algorithm>
#include <unordered_map>
#include "StringUtils.h"
#include "CSVReader.h"
#include "ArrayUtils.h"
#include "Predictor.h"


#define NONE_VALUE -99
#define DEBUG
using namespace std;




void read_targets(char *filename, unordered_map<string, size_t> &items, unordered_map<string, size_t> &users,
                  vector<vector<float>> &items_stats, vector<vector<float>> &users_stats,
                  vector<vector<string>> &targets,
                  vector<size_t> &target_users);

void read_ratings(const char *filename, unordered_map<string, size_t> &items, unordered_map<string, size_t> &users,
                  vector<vector<string>> &rows, vector<vector<float>> &items_stats,
                  vector<vector<float>> &users_stats);


void extract_norm_fvs(const unordered_map<string, size_t> &items, const unordered_map<string, size_t> &users,
                      const vector<vector<string>> &rows, float **users_fvs, vector<vector<float>> &items_stats,
                      vector<vector<float>> &users_stats);

void print_array(size_t M, size_t N,
                 float *const *users_fvs);

void init_array(const unordered_map<string, size_t> &items, const unordered_map<string, size_t> &users,
                float **users_fvs);

int main(int argc, char **argv) {
    if (argc < 4) {
        cout << "usage: ./TP1_Recsys ratings.csv targets.csv output.csv";
        exit(1);
    }
    unordered_map<string, size_t> items;
    unordered_map<string, size_t> users;
    vector<size_t> target_users;
    vector<vector<string> > targets;

    // The stats will hold the values for {count, avg, sum},
    vector<vector<float>> items_stats;
    vector<vector<float>> users_stats;

    vector<vector<string> > rows;


    read_ratings(argv[1], items, users, rows, items_stats, users_stats);

    read_targets(argv[2], items, users,
                 items_stats, users_stats,
                 targets, target_users);


    cout << "Rows: " << rows.size() << " items: " << items.size() << " users: " << users.size()
    << " Targets :" << target_users.size() << endl;
    cout << "Transposing  data..." << endl;

    // Computing items average
    compute_stats_avg(items_stats);
    compute_stats_avg(users_stats);


    float **users_fvs = alloc_2D_array<float>(users.size(), items.size());
    init_array(items, users, users_fvs);

    extract_norm_fvs(items, users, rows, users_fvs, items_stats, users_stats);



    // Computing user average after removing the item bias
//    compute_stats_avg(users_stats);


    cout << "Computing and Ranking Similiraties..." << endl;
    rows.clear();

    vector<vector<pair<size_t, float>>> ranking_user = rank_vectors(users_fvs, target_users, users.size(),
                                                                    items.size());




/*******/


    cout << "Pre-processing predictions..." <<
    endl;
    vector<float> predictions;
    vector<float> missing_predictions;
    user_predictions(items, users,
                     items_stats, users_stats,
                     ranking_user,
                     targets, predictions,
                     missing_predictions, users_fvs
    );
    delete
            users_fvs;
//    avg_predictions_personalized(items, users, items_stats, users_stats, targets, predictions, missing_predictions);

//    avg_predictions(items, users, items_stats, users_stats, targets, predictions, missing_predictions);

    items.

            clear();

    users.

            clear();

    items_stats.

            clear();

    cout << "Review predictions..." <<
    endl;
    review_predictions(targets, predictions, missing_predictions);


    cout << "Doing predictions..." <<
    endl;

    ofstream mixed_output(argv[3]);

    mixed_output << "itemId:userId,Prediction" <<
    endl;
    for (
            size_t index = 0;
            index < targets.

                    size();

            index++) {
        mixed_output << targets[index][0] << ':' << targets[index][1] << ','
        << predictions[index] <<
        endl;
    }

    mixed_output.

            close();

    return 0;
}


void init_array(const unordered_map<string, size_t> &items, const unordered_map<string, size_t> &users,
                float **users_fvs) {
    for (int user = 0; user < users.size(); user++) {
        for (int item = 0; item < items.size(); item++)
            users_fvs[user][item] = NONE_VALUE;
    }
}

void print_array(size_t M, size_t  N,
                 float *const *users_fvs) {
#ifdef DEBUG
    for (int row = 0; row < M; row++) {
        for (int col = 0; col < N; col++)
            cout << users_fvs[row][col] << "\t";
        cout << endl;
    }
#endif // DEBUG
}

void extract_norm_fvs(const unordered_map<string, size_t> &items, const unordered_map<string, size_t> &users,
                      const vector<vector<string>> &rows, float **users_fvs, vector<vector<float>> &items_stats,
                      vector<vector<float>> &users_stats) {
    float min_max[users.size()][2];
    for (int user = 0; user < users.size(); user++) {
        min_max[user][0] = 11;
        min_max[user][1] = -10;
    }
    for (size_t index = 0; index < rows.size(); index++) {
        size_t user_pos = users.at(rows[index][0]);
        size_t item_pos = items.at(rows[index][1]);
        float vote = stod(rows[index][2]);

        users_fvs[user_pos][item_pos] = vote;
    }
    print_array(users.size(), items.size(),users_fvs);

#ifdef BIAS
    cout << "Clean" << endl;
    print_array(users.size(), items.size(),users_fvs);

    for (int user = 0; user < users.size(); user++) {

        for (int item = 0; item < items.size(); item++) {

            float adjusted_vote = users_fvs[user][item] - users_stats[user][1];

            if (users_fvs[user][item] != NONE_VALUE) {
                if (adjusted_vote <= min_max[user][0])
                    min_max[user][0] = adjusted_vote;

                if (adjusted_vote >= min_max[user][1])
                    min_max[user][1] = adjusted_vote;


                users_fvs[user][item] = adjusted_vote;
            }
        }
    }

    cout << "Adjusted" << endl;
    print_array(users.size(), items.size(),users_fvs);

    for (int user = 0; user < users.size(); user++) {
        float min = min_max[user][0];
        float max = min_max[user][1];
        // Resetting the user Statistics during the normalization
//        users_stats[user][0] = 0;
//        users_stats[user][2] = 0;
        for (int item = 0; item < items.size(); item++) {
            if (users_fvs[user][item] == NONE_VALUE)
                users_fvs[user][item] = 0;
            else {
                users_fvs[user][item] = ((users_fvs[user][item] - min) / (max - min)) * 10;
//                users_stats[user][0] ++;
//                users_stats[user][2] += users_fvs[user][item];
            }
        }
        // Placing a new average
//        users_stats[user][1] = users_stats[user][2] / users_stats[user][0];
#ifdef DEBUG
        cout << "AVG user:" << user << ":" << users_stats[user][1] << endl;
#endif // DEBUG
    }


    cout << "normalized" << endl;
    print_array(users.size(), items.size(),users_fvs);
#endif // BIAS
}

void read_ratings(const char *filename, unordered_map<string, size_t> &items, unordered_map<string, size_t> &users,
                  vector<vector<string>> &rows, vector<vector<float>> &items_stats,
                  vector<vector<float>> &users_stats) {
    ifstream ratings_file(filename);
    CSVReader row_reader;
    size_t row_count = 0;
    // Skip the header
    ratings_file >> row_reader;
    while (ratings_file >> row_reader) {
        vector<string> item_user = split(row_reader[0], ':');
//        if ((items.find(item_user[1]) != items.end()))
        {
            rows.push_back(vector<string>({item_user[0], item_user[1], row_reader[1]}));
            row_count++;

            float vote = stod(row_reader[1]);

            if (items.find(item_user[1]) != items.end()) {
                size_t item_pos = items.at(item_user[1]);
                items_stats[item_pos][0]++;

                if (items_stats[item_pos][2] == -1)
                    items_stats[item_pos][2] = vote;
                else items_stats[item_pos][2] += vote;
            }
            else {
                items.insert({item_user[1], items.size()});
                items_stats.push_back(vector<float>({1, 0, vote}));
            }

            if ((users.find(item_user[0]) != users.end())) {
                size_t user_pos = users.at(item_user[0]);
                users_stats[user_pos][0]++;

                if (users_stats[user_pos][2] == -1)
                    users_stats[user_pos][2] = vote;
                else users_stats[user_pos][2] += vote;

            } else {
                users.insert({item_user[0], users.size()});
                users_stats.push_back(vector<float>({1, 0, vote}));
            }
        }
    }
    ratings_file.close();
}


void read_targets(char *filename, unordered_map<string, size_t> &items, unordered_map<string, size_t> &users,
                  vector<vector<float>> &items_stats, vector<vector<float>> &users_stats,
                  vector<vector<string>> &targets,
                  vector<size_t> &target_users) {
    CSVReader row_reader;
    cout << "Reading targets..." << endl;
    ifstream targets_file(filename);
    size_t item_count = 0, user_count = 0, target_count = 0;
    // Skip the header
    targets_file >> row_reader;

    while (targets_file >> row_reader) {

        targets.push_back(split(row_reader[0], ':'));

        if (items.find(targets[target_count][1]) == items.end()) {
            items.insert({targets[target_count][1], item_count++});
            items_stats.push_back(vector<float>({0, 0, -1}));
        }

        if (users.find(targets[target_count][0]) == users.end()) {
            target_users.push_back(user_count);
            users.insert({targets[target_count][0], user_count++});
            users_stats.push_back(vector<float>({0, 0, -1}));
        }
        target_count++;
    }

    targets_file.close();
}
