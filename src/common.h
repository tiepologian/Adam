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

#define ADAM_VERSION "0.1"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#if defined __clang__
#define COMPILER __VERSION__
#elif defined __GNUC__
#define COMPILER "GCC " STR(__GNUC__) "." STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__)
#else
#define COMPILER "UNKNOWN COMPILER"
#endif

#if defined __x86_64__
#define TARGET "x86_64"
#elif defined __arm64__
#define TARGET "arm64"
#else
#define TARGET ""
#endif

#define ADAM_VERSION_STR "Adam v" ADAM_VERSION " " COMPILER " " TARGET

namespace Utils
{
    static bool checkFileExists(std::string filename)
    {
        return std::filesystem::exists(std::filesystem::path(filename));
    }
    static std::size_t replace_all(std::string &inout, std::string_view what, std::string_view with)
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