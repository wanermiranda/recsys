#include <iostream>
#include <map>
#include <algorithm>
#include <unordered_map>
#include "StringUtils.h"
#include "CSVReader.h"
#include "ArrayUtils.h"
#include "Predictor.h"

using namespace std;

//#define BIAS


void read_targets(char *filename, unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                  vector<vector<double>> &users_stats, vector<vector<double>> &items_stats,
                  vector<vector<string>> &targets);

void read_ratings(const char *filename, unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                  vector<vector<string>> &rows, vector<vector<double>> &users_stats,
                  vector<vector<double>> &items_stats);

void extract_fvs(const unordered_map<string, size_t> &users, const unordered_map<string, size_t> &items,
                 const vector<vector<string>> &rows, double *const *items_fvs, vector<vector<double>> &users_stats,
                 vector<vector<double>> &items_stats);

int main(int argc, char **argv) {
    if (argc < 4) {
        cout << "usage: ./TP1_Recsys ratings.csv targets.csv output.csv";
        exit(1);
    }
    unordered_map<string, size_t> users;
    unordered_map<string, size_t> items;
    vector<vector<string> > targets;

    // The stats will hold the values for {count, avg, sum},
    vector<vector<double>> users_stats;
    vector<vector<double>> items_stats;

    vector<vector<string> > rows;

    read_targets(argv[2], users, items, users_stats, items_stats, targets);


    read_ratings(argv[1], users, items, rows, users_stats, items_stats);

    cout << "Rows: " << rows.size() << " Users: " << users.size() << " Items: " << items.size() << endl;
    cout << "Transposing  data..." << endl;

    // Computing users average
    compute_stats_avg(users_stats);
    compute_stats_avg(items_stats);


    double **items_fvs = alloc_2D_array<double>(items.size(), users.size());
    extract_fvs(users, items, rows, items_fvs, users_stats, items_stats);

    // Computing item average after removing the user bias
//    compute_stats_avg(items_stats);


    cout << "Computing and Ranking Similiraties..." << endl;
    rows.clear();

    vector<vector<size_t> > ranking_item = rank_vectors(items_fvs, items.size(), users.size());

    delete items_fvs;

    cout << "Pre-processing predictions..." << endl;
    vector<double> predictions;
    vector<double> missing_predictions;
    item_predictions(users, items, users_stats, items_stats, ranking_item, targets, predictions, missing_predictions);

//    avg_predictions_personalized(users, items, users_stats, items_stats, targets, predictions, missing_predictions);

    avg_predictions(users, items, users_stats, items_stats, targets, predictions, missing_predictions);

    users.clear();
    items.clear();
    users_stats.clear();

    cout << "Review predictions..." << endl;
    review_predictions(targets, predictions, missing_predictions);


    cout << "Doing predictions..." << endl;
    ofstream mixed_output(argv[3]);
    mixed_output << "UserId:ItemId,Prediction" << endl;
    for (size_t index = 0; index < targets.size(); index++) {
        mixed_output << targets[index][0] << ':' << targets[index][1] << ','
        << predictions[index] << endl;
    }

    mixed_output.close();
    return 0;
}

void extract_fvs(const unordered_map<string, size_t> &users, const unordered_map<string, size_t> &items,
                 const vector<vector<string>> &rows, double *const *items_fvs, vector<vector<double>> &users_stats,
                 vector<vector<double>> &items_stats) {
    for (size_t index = 0; index < rows.size(); index++) {
        size_t user_pos = users.at(rows[index][0]);
        size_t item_pos = items.at(rows[index][1]);
#ifdef BIAS
        double vote = stod(rows[index][2]) - users_stats[user_pos][1];
#else
        double vote = stod(rows[index][2]);
#endif //BIAS


//        users_stats[user_pos][2] += vote;
//        items_stats[item_pos][2] += vote;

        items_fvs[item_pos][user_pos] = vote;
    }
}

void read_ratings(const char *filename, unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                  vector<vector<string>> &rows, vector<vector<double>> &users_stats,
                  vector<vector<double>> &items_stats) {
    ifstream ratings_file(filename);
    CSVReader row_reader;
    size_t row_count = 0;
    // Skip the header
    ratings_file >> row_reader;
    while (ratings_file >> row_reader) {
        vector<string> user_item = split(row_reader[0], ':');

        if ((users.find(user_item[0]) != users.end()) || (items.find(user_item[1]) != items.end())) {
            rows.push_back(vector<string>({user_item[0], user_item[1], row_reader[1]}));
            row_count++;

            double vote = stod(row_reader[1]);

            if (users.find(user_item[0]) != users.end()) {
                size_t user_pos = users.at(user_item[0]);
                users_stats[user_pos][0]++;

                if (users_stats[user_pos][2] == -1)
                    users_stats[user_pos][2] = vote;
                else users_stats[user_pos][2] += vote;
            }else{
                users.insert({user_item[0], users.size()});
                users_stats.push_back(vector<double>({1, 0, vote}));
            }

            if ((items.find(user_item[1]) != items.end())) {
                size_t item_pos = items.at(user_item[1]);
                items_stats[item_pos][0]++;

                if (items_stats[item_pos][2] == -1)
                    items_stats[item_pos][2] = vote;
                else items_stats[item_pos][2] += vote;

            } else {
                items.insert({user_item[1], items.size()});
                items_stats.push_back(vector<double>({1, 0, vote}));
            }
        }
    }
    ratings_file.close();
}


void read_targets(char *filename, unordered_map<string, size_t> &users, unordered_map<string, size_t> &items,
                  vector<vector<double>> &users_stats, vector<vector<double>> &items_stats,
                  vector<vector<string>> &targets) {
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
            users_stats.push_back(vector<double>({0, 0, -1}));
        }

        if (items.find(targets[target_count][1]) == items.end()) {
            items.insert({targets[target_count][1], item_count++});
            items_stats.push_back(vector<double>({0, 0, -1}));
        }
        target_count++;
    }

    targets_file.close();
}
