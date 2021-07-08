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
