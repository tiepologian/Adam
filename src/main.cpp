#include "adam.h"
#include "cxxopts.hpp"

/*
 * @TODO Print header
 * @TODO Add license
 * @TODO Specify specific table
 * @TODO Add verbose mode
 * @TODO Add --new-only mode
 * @TODO Transition ios makefile to Theos
 */

int main(int argc, char **argv)
{
    cxxopts::Options options("Adam", "Apple Database Async Monitor");
    options.add_options()("d,database", "Specify database to monitor", cxxopts::value<std::vector<std::string>>(), "FILE")("t,table", "Only monitor specific database table", cxxopts::value<std::string>())("o,output", "Specify output JSON file", cxxopts::value<std::string>()->default_value("output.json"))("l,live", "Enable live mode", cxxopts::value<bool>()->default_value("false"))("h,help", "Print usage");

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