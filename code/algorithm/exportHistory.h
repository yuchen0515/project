#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <fstream>

class History {
    public:
        std::string getCurrentSystemTime();
        void openFile();
        void recordStep(std::pair<int32_t, int32_t> ori,
                std::pair<int32_t, int32_t> det);

        void closeFile();


    private:
        int32_t count = 0;
        char date[80] = {0};
        std::ofstream outFile;
};
