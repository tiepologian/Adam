#include <dtl/dtl.hpp>
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

using dtl::Diff;
using dtl::elemInfo;
using dtl::uniHunk;
using json = nlohmann::json;

namespace Utils
{
    static bool checkFileExists(std::string filename)
    {
        return std::filesystem::exists(std::filesystem::path(filename));
    }
    std::size_t replace_all(std::string &inout, std::string_view what, std::string_view with)
    {
        std::size_t count{};
        for (std::string::size_type pos{};
             inout.npos != (pos = inout.find(what.data(), pos, what.length()));
             pos += with.length(), ++count)
        {
            inout.replace(pos, what.length(), with.data(), with.length());
        }
        return count;
    }
    static std::string extractTableName(const std::string &q)
    {
        std::regex r("\"([^\"]+)\"");
        std::smatch m;
        std::regex_search(q, m, r);
        return m[1];
    }
    static std::string extractQueryType(std::string &q)
    {
        char *token = strtok(q.data(), " ");
        return token;
    }
    static std::string extractQueryData(std::string &q)
    {
        std::regex r("\\(([^\"]+)\\)");
        std::smatch m;
        std::regex_search(q, m, r);
        std::string qData = m[1];
        Utils::replace_all(qData, "\'", "\"");
        Utils::replace_all(qData, "NULL", "null");
        return std::string("[" + qData + "]");
    }
    static json unifiedDiff(std::string fp1, std::string fp2)
    {
        typedef std::string elem;
        typedef std::vector<elem> sequence;

        std::ifstream Aifs(fp1.c_str());
        std::ifstream Bifs(fp2.c_str());
        elem buf;
        sequence ALines, BLines;

        while (getline(Aifs, buf))
        {
            ALines.push_back(buf);
        }
        while (getline(Bifs, buf))
        {
            BLines.push_back(buf);
        }

        Diff<elem> diff(ALines, BLines);
        diff.onHuge();
        diff.compose();

        Aifs.close();
        Bifs.close();

        json dbJson = json::array();

        diff.composeUnifiedHunks();
        for (auto i : diff.getUniHunks())
        {
            for (auto j : i.change)
            {
                if (j.second.type != 0)
                {
                    std::string query = j.first;
                    std::string table = Utils::extractTableName(query);
                    std::string queryType = (j.second.type == 1) ? Utils::extractQueryType(query) : "DELETE";
                    std::string queryValues = Utils::extractQueryData(query);
                    //std::cout  << queryType << " > " << table << std::endl;
                    //std::cout << queryValues << std::endl;
                    json j;
                    j["QueryType"] = queryType;
                    j["Table"] = table;
                    j["Values"] = json::parse(queryValues);
                    dbJson.push_back(j);
                }
            }
        }
        return dbJson;
    }
}