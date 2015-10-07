#include <iostream>
#include <map>
#include <algorithm>
#include <unordered_map>
#include "StringUtils.h"
#include "CSVReader.h"
#include "ArrayUtils.h"
#include "Predictor.h"


using namespace std;




void read_targets(char *filename, unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                  vector<vector<float>> &users_stats, vector<vector<float>> &items_stats,
                  vector<vector<string>> &targets,
                  vector<size_t> &target_items);

void read_ratings(const char *filename, unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                  vector<vector<string>> &rows, vector<vector<float>> &users_stats,
                  vector<vector<float>> &items_stats);


void extract_fvs(const unordered_map<string, size_t> &users, const unordered_map<string, size_t> &items,
                 const vector<vector<string>> &rows, float **items_fvs, vector<vector<float>> &users_stats,
                 vector<vector<float>> &items_stats);

void print_array(const unordered_map<string, size_t> &users, const unordered_map<string, size_t> &items,
                 float *const *items_fvs);

void init_array(const unordered_map<string, size_t> &users, const unordered_map<string, size_t> &items,
                float **items_fvs);

int main(int argc, char **argv) {
    if (argc < 4) {
        cout << "usage: ./TP1_Recsys ratings.csv targets.csv output.csv";
        exit(1);
    }
    unordered_map<string, size_t> users;
    unordered_map<string, size_t> items;
    vector<size_t> target_items;
    vector<vector<string> > targets;

    // The stats will hold the values for {count, avg, sum},
    vector<vector<float>> users_stats;
    vector<vector<float>> items_stats;

    vector<vector<string> > rows;

    read_targets(argv[2], users, items,
                 users_stats, items_stats,
                 targets, target_items);


    read_ratings(argv[1], users, items, rows, users_stats, items_stats);

    cout << "Rows: " << rows.size() << " Users: " << users.size() << " Items: " << items.size()
    << " Targets :" << target_items.size() << endl;
    cout << "Transposing  data..." << endl;

    // Computing users average
    compute_stats_avg(users_stats);
    compute_stats_avg(items_stats);


    float **items_fvs = alloc_2D_array<float>(items.size(), users.size());
    init_array(users, items, items_fvs);

    extract_fvs(users, items, rows, items_fvs, users_stats, items_stats);



    // Computing item average after removing the user bias
//    compute_stats_avg(items_stats);


    cout << "Computing and Ranking Similiraties..." << endl;
    rows.clear();

    vector<vector<pair<size_t, float>>> ranking_item = rank_vectors(items_fvs, target_items, items.size(),
                                                                    users.size());




/*******/


    cout << "Pre-processing predictions..." <<
    endl;
    vector<float> predictions;
    vector<float> missing_predictions;
    item_predictions(users, items,
                     users_stats, items_stats,
                     ranking_item,
                     targets, predictions,
                     missing_predictions, items_fvs
    );
    delete
            items_fvs;
//    avg_predictions_personalized(users, items, users_stats, items_stats, targets, predictions, missing_predictions);

//    avg_predictions(users, items, users_stats, items_stats, targets, predictions, missing_predictions);

    users.

            clear();

    items.

            clear();

    users_stats.

            clear();

    cout << "Review predictions..." <<
    endl;
    review_predictions(targets, predictions, missing_predictions);


    cout << "Doing predictions..." <<
    endl;

    ofstream mixed_output(argv[3]);

    mixed_output << "UserId:ItemId,Prediction" <<
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


void init_array(const unordered_map<string, size_t> &users, const unordered_map<string, size_t> &items,
                float **items_fvs) {
    for (int item = 0; item < items.size(); item++) {
        for (int user = 0; user < users.size(); user++)
            items_fvs[item][user] = 0.0;
    }
}

void print_array(const unordered_map<string, size_t> &users, const unordered_map<string, size_t> &items,
                 float *const *items_fvs) {
    for (int item = 0; item < items.size(); item++) {
        for (int user = 0; user < users.size(); user++)
            cout << items_fvs[item][user] << "\t";
        cout << endl;
    }
}

void extract_fvs(const unordered_map<string, size_t> &users, const unordered_map<string, size_t> &items,
                 const vector<vector<string>> &rows, float **items_fvs, vector<vector<float>> &users_stats,
                 vector<vector<float>> &items_stats) {
    float min_max[items.size()][2];
    for (int item = 0; item < items.size(); item++) {
        min_max[item][0] = 11;
        min_max[item][1] = -10;
    }
    for (size_t index = 0; index < rows.size(); index++) {
        size_t user_pos = users.at(rows[index][0]);
        size_t item_pos = items.at(rows[index][1]);
        float vote = stod(rows[index][2]);

        items_fvs[item_pos][user_pos] = vote;
    }

#ifdef BIAS
    cout << "Clean" << endl;
    print_array(users, items, items_fvs);

    for (int item = 0; item < items.size(); item++) {

        for (int user = 0; user < users.size(); user++) {

            float adjusted_vote = items_fvs[item][user] - users_stats[user][1];

            if (adjusted_vote <= min_max[item][0])
                min_max[item][0] = adjusted_vote;

            if (adjusted_vote >= min_max[item][1])
                min_max[item][1] = adjusted_vote;

            items_fvs[item][user] = adjusted_vote;
        }
    }

    cout << "Adjusted" << endl;
    print_array(users, items, items_fvs);

    for (int item = 0; item < items.size(); item++) {
        float min = min_max[item][0];
        float max = min_max[item][1];
        for (int user = 0; user < users.size(); user++) {
            items_fvs[item][user] = ((items_fvs[item][user] - min) / (max - min)) * 10;
        }
    }
    cout << "normalized" << endl;
    print_array(users, items, items_fvs);
#endif
}

void read_ratings(const char *filename, unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                  vector<vector<string>> &rows, vector<vector<float>> &users_stats,
                  vector<vector<float>> &items_stats) {
    ifstream ratings_file(filename);
    CSVReader row_reader;
    size_t row_count = 0;
    // Skip the header
    ratings_file >> row_reader;
    while (ratings_file >> row_reader) {
        vector<string> user_item = split(row_reader[0], ':');
//        if ((items.find(user_item[1]) != items.end()) || (users.find(user_item[0]) != users.end()))
        {
            rows.push_back(vector<string>({user_item[0], user_item[1], row_reader[1]}));
            row_count++;

            float vote = stod(row_reader[1]);

            if (users.find(user_item[0]) != users.end()) {
                size_t user_pos = users.at(user_item[0]);
                users_stats[user_pos][0]++;

                if (users_stats[user_pos][2] == -1)
                    users_stats[user_pos][2] = vote;
                else users_stats[user_pos][2] += vote;
            }
            else {
                users.insert({user_item[0], users.size()});
                users_stats.push_back(vector<float>({1, 0, vote}));
            }

            if ((items.find(user_item[1]) != items.end())) {
                size_t item_pos = items.at(user_item[1]);
                items_stats[item_pos][0]++;

                if (items_stats[item_pos][2] == -1)
                    items_stats[item_pos][2] = vote;
                else items_stats[item_pos][2] += vote;

            } else {
                items.insert({user_item[1], items.size()});
                items_stats.push_back(vector<float>({1, 0, vote}));
            }
        }
    }
    ratings_file.close();
}


void read_targets(char *filename, unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                  vector<vector<float>> &users_stats, vector<vector<float>> &items_stats,
                  vector<vector<string>> &targets,
                  vector<size_t> &target_items) {
    CSVReader row_reader;
    cout << "Reading targets..." << endl;
    ifstream targets_file(filename);
    size_t user_count = 0, item_count = 0, target_count = 0;
    // Skip the header
    targets_file >> row_reader;

    while (targets_file >> row_reader) {

        targets.push_back(split(row_reader[0], ':'));

        if (users.find(targets[target_count][0]) == users.end()) {
            users.insert({targets[target_count][0], user_count++});
            users_stats.push_back(vector<float>({0, 0, -1}));
        }

        if (items.find(targets[target_count][1]) == items.end()) {
            target_items.push_back(item_count);
            items.insert({targets[target_count][1], item_count++});
            items_stats.push_back(vector<float>({0, 0, -1}));
        }
        target_count++;
    }

    targets_file.close();
}
