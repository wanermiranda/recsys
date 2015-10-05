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

#define NN 5
#define NTHREADS 8

std::mutex mtx;

auto distance_comparer = [](const std::pair<float, int>& a, const std::pair<float, int>& b) {
    return (a.first < b.first && a.second != b.second);
};

void rank_vector(const vector<vector<short>> &feature_vectors, int ind_ext, vector< vector<int>> &results);

float cosine_distance(vector<short> vector1, vector<short> vector2) {
    float sum_rv1_rv2 = 0, sum_pow_rv1 = 0, sum_pow_rv2 = 0;

    for (int index = 0; index < vector1.size(); index ++){
        sum_rv1_rv2 += vector1[index] * vector2[index];
        sum_pow_rv1 += pow(vector1[index],2);
        sum_pow_rv2 += pow(vector2[index],2);
    }

    return 1 - (sum_rv1_rv2/(sum_pow_rv1 * sum_pow_rv2));;
}

float manhattan_distance(vector<short> vector1, vector<short> vector2) {
    float dist = 0;

    for (int index = 0; index < vector1.size(); index ++){
        dist += abs(vector1[index] - vector2[index]);
    }
    return dist;
}

vector< vector<int> > rank_vectors(vector<vector<short> > feature_vectors){
    vector< vector<int> > results;
    std::thread threads_active[NTHREADS];
    int threads_count = 0;
    for (int ind_ext = 0; ind_ext < feature_vectors.size(); ind_ext++) {
        cout << "Computing distances for user: " << ind_ext + 1 << endl;
        threads_active[threads_count] = std::thread(rank_vector, std::ref(feature_vectors), ind_ext, std::ref(results));
        threads_count ++;
        if (threads_count >= NTHREADS) {
            threads_count = 0;
            for (int i = 0; i < NTHREADS; i++)
                threads_active[i].join();
        } else if (ind_ext+1 == feature_vectors.size()) {
            for (int i = 0; i < threads_count; i++)
                threads_active[i].join();
        }
    }
    return results;
}

void rank_vector(const vector<vector<short>> &feature_vectors, int ind_ext, vector< vector<int>> &results) {
    multiset<pair<float, int>, decltype(distance_comparer)> distances(distance_comparer);
    for (int ind_int = 0; ind_int < feature_vectors.size(); ind_int++) {
            if (ind_ext != ind_int) // skipping itself
                distances.insert(
                        make_pair(ind_int, manhattan_distance(feature_vectors[ind_ext], feature_vectors[ind_int])));
        }
    vector<int> rank;
    int counter = 0;
    for (auto const &e : distances) {
            rank.push_back(e.second);
            if (counter++ > NN) break;
        }

    mtx.lock();
    results.push_back(rank);
    mtx.unlock();
}

int main(int argc,  char** argv) {
    if (argc < 4) {
        cout << "usage: ./TP1_Recsys ratings.csv targets.csv output.csv";
        exit(1);
    }
    ifstream ratings_file(argv[1]);
    unordered_map<string,int> users;
    unordered_map<string,int> items;

    // The stats will hold the values for {count, avg|sum, std_dev|sum},
    // at first std_dev and avg will hold the sum of variances and votes util the computing phase
    vector<vector<float>> users_stats;

    vector< vector<string> > rows;
    int user_count = 0, item_count = 0, row_count = 0;

    CSVReader row_reader;
    // Skip the header
    ratings_file >> row_reader;
    while(ratings_file >> row_reader)
    {
        vector<string> user_item = split(row_reader[0], ':');
        rows.push_back(vector<string>({user_item[0], user_item[1], row_reader[1]}));
        row_count++;
        short vote = stoi(row_reader[1]);

        if (users.find(user_item[0]) == users.end()) {
            users.insert({user_item[0], user_count++});
            users_stats.push_back(vector<float>({1,float(vote),0}));
        }
        else {
            int user_pos = users.at(user_item[0]);
            users_stats[user_pos][0]++;
            users_stats[user_pos][1] = users_stats[user_pos][1] + vote;
        }


        if (items.find(user_item[1]) == items.end())
            items.insert({user_item[1], item_count++});
    }
    ratings_file.close();


    // Averaging
    for (int index = 0; index < users_stats.size(); index ++)
        users_stats[index][1] = float(users_stats[index][1])/float(users_stats[index][0]);

    // Finishing the std deviation
    for (int index = 0; index < users_stats.size(); index ++)
        users_stats[index][2] = sqrt(pow(float(users_stats[index][2] - users_stats[index][0]), 2)
                                     / float(users_stats[index][0]-1));



    cout << "Rows: " << rows.size() << " Users: " << users.size() << " Items: " << items.size() << endl;
    cout << "Transposing  data..." << endl;
    vector<vector<short>> users_fvs(users.size(), vector<short>(items.size()));

    for (int index = 0; index < rows.size(); index++){
        int user_pos = users.at(rows[index][0]);
        int item_pos = items.at(rows[index][1]);
        users_fvs[user_pos][item_pos] = stoi(rows[index][2]);
    }
    cout << "Computing and Ranking Similiraties..." << endl;
    rows.clear();

    vector< vector<int> > ranking = rank_vectors(users_fvs);

    cout << "Reading targets..." << endl;
    ifstream targets_file(argv[2]);
    vector< vector<string> > targets;

    // Skip the header
    targets_file >> row_reader;
    while(targets_file >> row_reader)
        targets.push_back(split(row_reader[0],':'));

    targets_file.close();

    cout << "Pre-processing predictions..." << endl;

    vector< float > predictions;
    vector< float > missing_predictions;
    for (int index = 0; index < targets.size(); index++){
        if (users.find(targets[index][0]) != users.end()) {
            int target_user_pos = users.at(targets[index][0]);
            float avg = 0;

            // Capturing the average rating from the nearest neighbors
            for (int vect_index = 0; vect_index < NN; vect_index ++) {
                int user_pos = ranking[target_user_pos][vect_index];
                avg += float(users_stats[user_pos][1]);
            }
            // Average them
            avg /= NN;
            predictions.push_back(avg);
        }
        else {
            predictions.push_back(0);
            missing_predictions.push_back(index);
        }

    }


    users.clear();
    items.clear();
    users_stats.clear();

    cout << "Review predictions..." << endl;

    // From the missing predictions, speculate a possible value based on the average ratings same users or items
    for (int missing_index = 0; missing_index < missing_predictions.size(); missing_index++){
        int target_pos = missing_predictions[missing_index];
        string user_id = targets[target_pos][0];
        string item_id = targets[target_pos][1];
        int item_count = 0, user_count = 0;
        float item_avg = 0, user_avg = 0;
        for (int target_index = 0; target_index < targets.size(); target_index ++) {
            if (targets[target_index][0] == user_id) {
                user_avg += predictions[target_index];
                user_count ++;
            }
            if (targets[target_index][1] == item_id) {
                item_avg += predictions[target_index];
                item_count ++;
            }
        }

        if (item_count > 0) {
            item_avg /= item_count;
            predictions[target_pos] = item_avg;
        }
        else if (user_count > 0) {
            user_avg /= user_count;
            predictions[target_pos] = user_avg;
        }
        else predictions[target_pos] = 5;
    }



    cout << "Doing predictions..." << endl;
    ofstream mixed_output("/home/gorigan/ClionProjects/TP1_Recsys/mixed_out.csv");
    mixed_output << "UserId:ItemId,Prediction" << endl;
    for (int index = 0; index < targets.size(); index++){
            mixed_output << targets[index][0] << ':' << targets[index][1] << ','
            << predictions[index] << endl;
    }

    mixed_output.close();
    return 0;
}
