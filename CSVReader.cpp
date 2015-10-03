#include "CSVReader.h"


void CSVReader::read_line(std::ifstream& str) {
    std::string line;
    std::getline(str, line);

    std::stringstream lineStream(line);
    std::string cell;

    m_data.clear();
    while (std::getline(lineStream, cell, ',')) {
        m_data.push_back(cell);
    }
}

std::vector<std::string> CSVReader::last_line()
{
    return m_data;
}

std::ifstream& operator>>(std::ifstream& str,CSVReader& data)
{
    data.read_line(str);
    return str;
}