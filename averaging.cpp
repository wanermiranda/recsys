#include<iostream>
#include <map>
#include "StringUtils.h"
#include "CSVReader.h"
#include <unordered_map>
#include <algorithm>


using namespace std;

int main_averaging(int argc,  char** argv) {
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
    vector<vector<float>> items_stats;
    vector< vector<string> > rows;
    int user_count = 0, item_count = 0, row_count = 0;

    CSVReader row_reader;
    // Skip the header
    ratings_file >> row_reader;
    while(ratings_file >> row_reader)
    {
        vector<string> user_item = split(row_reader[0], ':');
        short vote = stoi(row_reader[1]);
        rows.push_back(vector<string>({user_item[0], user_item[1], row_reader[1]}));
        row_count++;

        if (users.find(user_item[0]) == users.end()) {
            users.insert({user_item[0], user_count++});
            users_stats.push_back(vector<float>({1,float(vote),0}));
        }
        else {
            int user_pos = users.at(user_item[0]);
            users_stats[user_pos][0]++;
            users_stats[user_pos][1] = users_stats[user_pos][1] + vote;
        }

        if (items.find(user_item[1]) == items.end()) {
            items.insert({user_item[1], item_count++});
            items_stats.push_back(vector<float>({1,float(vote),0}));
        }
        else {
            int item_pos = items.at(user_item[1]);
            items_stats[item_pos][0] ++;
            items_stats[item_pos][1] = items_stats[item_pos][1] + vote;
        }
    }
    ratings_file.close();


    cout << "Rows: " << rows.size() << " Users: " << users.size() << " Items: " << items.size() << endl;
    cout << "Transposing  data..." << endl;
    vector< vector <short> > user_item_ratings(users.size(), vector<short>(items.size()));



    // Averaging
    for (int index = 0; index < users_stats.size(); index ++)
        users_stats[index][1] = float(users_stats[index][1])/float(users_stats[index][0]);;

    for (int index = 0; index < items_stats.size(); index ++)
        items_stats[index][1] = float(items_stats[index][1])/float(items_stats[index][0]);;

//    // Finishing the std deviation
//    for (int index = 0; index < users_stats.size(); index ++)
//        users_stats[index][2] = sqrt(float(users_stats[index][2] - users_stats[index][0])/float(users_stats[index][0]));
//
//    for (int index = 0; index < items_stats.size(); index ++)
//        items_stats[index][2] = sqrt(float(items_stats[index][2] - items_stats[index][0])/float(items_stats[index][0]));

    cout << "Matrix Built." << endl;
    rows.clear();

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
        if (items.find(targets[index][1]) != items.end()) {
            int item_pos = items.at(targets[index][1]);
            predictions.push_back(items_stats[item_pos][1]);
        }
        else if (users.find(targets[index][0]) != users.end()) {
            int user_pos = users.at(targets[index][0]);
            predictions.push_back(users_stats[user_pos][1]);
        }
        else {
            predictions.push_back(0);
            missing_predictions.push_back(index);
        }
    }


    users.clear();
    items.clear();
    users_stats.clear();
    items_stats.clear();

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
    }

    cout << "Pre-processing predictions..." << endl;


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
