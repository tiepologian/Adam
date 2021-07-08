/* 
 * MIT License
 *
 * Copyright (c) 2021 Gianluca Tiepolo
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "adam.h"
#include "cxxopts.hpp"

int main(int argc, char **argv)
{
    cxxopts::Options options("Adam", "Apple Database Async Monitor");
    options.add_options()("d,database", "Database to monitor", cxxopts::value<std::vector<std::string>>(), "FILE")("t,table", "Optional table to monitor", cxxopts::value<std::string>(),"TABLE")("o,output", "Write output to file", cxxopts::value<std::string>()->default_value("output.json"),"FILE")("l,live", "Enable live mode", cxxopts::value<bool>()->default_value("false"))("h,help", "Print usage");

    auto result = options.parse(argc, argv);
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    if (!result.count("database"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    Adam::Adam adm;
    std::string outputPath = result["output"].as<std::string>();
    adm.setOutputFile(std::filesystem::absolute(outputPath));
    adm.setLiveMode(result["live"].as<bool>());

    if (result.count("table"))
    {
        if (result.count("database") > 1)
        {
            std::cout << options.help() << std::endl;
            exit(0);
        }
        adm.setTableMode(result["table"].as<std::string>());
    }

    for (auto i : result["database"].as<std::vector<std::string>>())
    {
        if (Utils::checkFileExists(i))
            adm.addFile(std::filesystem::absolute(i));
        else
        {
            std::cout << "[*] ERROR: file doesn't exist " << std::filesystem::absolute(i) << std::endl;
            exit(1);
        }
    }

    adm.run();
    return 0;
}