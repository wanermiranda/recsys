//
// Created by gorigan on 10/2/15.
//

#ifndef TP1_RECSYS_CSVREADER_H
#define TP1_RECSYS_CSVREADER_H
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

class CSVReader {
public:
    std::string const& operator[](std::size_t index) const
    {
        if (index > size()) {
            std::cout << "Index out of bounds." << std::endl;
            return NULL;
        }
        else return m_data[index];


    }
    std::size_t size() const
    {
        return m_data.size();
    }

    std::vector<std::string> last_line();

    void read_line(std::ifstream& str);
private:
    std::vector<std::string> m_data;
};

std::ifstream& operator>>(std::ifstream& str,CSVReader& data);
#endif //TP1_RECSYS_CSVREADER_H
