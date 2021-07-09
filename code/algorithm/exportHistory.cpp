#include "exportHistory.h"

std::string History::getCurrentSystemTime() {
    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime(&time);
    sprintf(date, "%d-%02d-%02d_%02d.%02d.%02d",
            (int32_t)ptm->tm_year + 1900,
            (int32_t)ptm->tm_mon + 1,
            (int32_t)ptm->tm_mday,
            (int32_t)ptm->tm_hour,
            (int32_t)ptm->tm_min,
            (int32_t)ptm->tm_sec);
    return std::string(date);
}

void History::openFile() {
    outFile.open(std::string(date) + ".txt");
    outFile << "<---- Start a new round ---->" << std::endl;
}

void History::importFile(std::string fileName) {
    importFile_.open(fileName + ".txt");

    std::string TEMP;
    while (!importFile_.eof()) {
        std::getline(importFile_, TEMP);
        std::stringstream str(TEMP);
        std::string cmp;

        str >> cmp;
        if (cmp != "Step") {
            continue;
        }

        str >> cmp;
        str >> cmp;

        str >> cmp;
        std::pair<int32_t, int32_t> ori;
        ori.first = cmp[0] - '1';
        ori.second = cmp[1] - 'A';

        str >> cmp;


        str >> cmp;
        std::pair<int32_t, int32_t> des;
        des.first = cmp[0] - '1';
        des.second = cmp[1] - 'A';

        story.push({ori, des});
    }
}

void History::recordStep(std::pair<int32_t, int32_t> ori,
        std::pair<int32_t, int32_t> det) {
    auto [ori_x, ori_y] = ori;
    auto [det_x, det_y] = det;

    if (count % 2 == 0) {
        outFile <<  "<First hand>" << std::endl;
    } else {
        outFile <<  "<Second hand>" << std::endl;
    }

    outFile << "Step " << count + 1 << ": from ";
    outFile << (char)(ori_x + '1') << (char)(ori_y + 'A') << " to ";
    outFile << (char)(det_x + '1') << (char)(det_y + 'A') << std::endl;
    outFile << std::endl;

    count += 1;
}

void History::closeFile() {
    outFile.close();
}


bool History::getStep(std::pair<int32_t, int32_t> &ori,
        std::pair<int32_t, int32_t> &des) {
    if (story.empty()) {
        return false;
    }

    auto res = story.front();
    story.pop();

    ori = res.first;
    des = res.second;

    return true;
}
