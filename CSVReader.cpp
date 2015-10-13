#include "CSVReader.h"


void CSVReader::read_line(std::ifstream& str) {
    std::string line;
    std::getline(str, line);

    std::stringstream line_stream(line);
    std::string cell;

    _current_line.clear();
    while (std::getline(line_stream, cell, ',')) {
        _current_line.push_back(cell);
    }
}

std::vector<std::string> CSVReader::last_line()
{
    return _current_line;
}

std::ifstream& operator>>(std::ifstream& str,CSVReader& data)
{
    data.read_line(str);
    return str;
}