#include <iostream>
#include <fstream>
#include <algorithm>
#include "set"
#include "map"

std::set<std::string> split(const std::string &str) {
    std::set<std::string> bigramms;
    for (int i = 0; i < str.length() - 1; i++)
        bigramms.insert(str.substr(i, 2));
    if (bigramms.empty()) bigramms.insert(str);
    return bigramms;
}

double ratio(const std::set<std::string> &v, const std::set<std::string> &w) {
    std::set<std::string> inter;
    set_intersection(v.begin(), v.end(), w.begin(), w.end(), inserter(inter, inter.begin()));
    return (double) inter.size() / ((double) v.size() + (double) w.size() - (double) inter.size());
}

std::string correct(const std::string &str, const std::map<std::string, std::pair<int, std::set<std::string>>> &dict) {
    double cur_ratio = -1;
    int cur_freq = 0;
    std::set<std::string> bigramms = split(str);
    std::string right_word = "";
    for (std::pair<std::string, std::pair<int, std::set<std::string>>> note : dict) {
        double step_ratio = ratio(bigramms, note.second.second);
        int step_freq = note.second.first;
        if (step_ratio > cur_ratio || (step_ratio == cur_ratio && step_freq > cur_freq) ||
            (step_ratio == cur_ratio && step_freq == cur_freq && note.first < right_word)) {
            right_word = note.first;
            cur_ratio = step_ratio;
            cur_freq = step_freq;
        }
    }
    return right_word;
}

int main() {
    std::ifstream f("count_big.txt");
    std::map<std::string, std::pair<int, std::set<std::string>>> dict;
    std::string word;
    int freq;
    while (!f.eof()) {
        f >> word;
        f >> freq;
        dict[word] = make_pair(freq, split(word));
    }
    f.close();

    while (getline(std::cin, word)) {
        std::cout << correct(word, dict) << std::endl;
    }
}