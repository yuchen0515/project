#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <queue>

class History {
    public:
        std::string getCurrentSystemTime();
        void openFile();
        void openFile(std::string fileName);
        void recordStep(std::pair<int32_t, int32_t> ori,
                std::pair<int32_t, int32_t> det);

        void closeFile();

        void importFile(std::string fileName);
        bool getStep(std::pair<int32_t, int32_t> &ori,
                std::pair<int32_t, int32_t> &des);


    private:
        int32_t count = 0;
        char date[80] = {0};
        std::ofstream outFile;
        std::ifstream importFile_;
        std::queue<std::pair<std::pair<int32_t, int32_t>,
            std::pair<int32_t, int32_t>>> story;
};
