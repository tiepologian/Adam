#include <chrono>
#include <thread>
#include <unistd.h>
#include <csignal>
#include "sql-wrapper.h"
#include "common.h"

#ifndef ADAMPP
#define ADAMPP

namespace Adam
{
    class Adam
    {
    public:
        Adam()
        {
            this->isLive_ = false;
            std::filesystem::create_directory(std::filesystem::temp_directory_path() / "adam-tmp");
        }
        virtual ~Adam()
        {
            std::filesystem::remove_all(std::filesystem::temp_directory_path() / "adam-tmp");
        }
        void addFile(std::string f)
        {
            this->dbPath_.push_back(f);
        }
        void setLiveMode(bool live)
        {
            this->isLive_ = live;
        }
        void setOutputFile(std::string s)
        {
            Adam::outPath_ = s;
            std::cout << "[*] Writing output to " << s << std::endl;
        }
        static void signalHandler(int signal __attribute__((unused)))
        {
            json result;
            for (auto i : wrappers_)
            {
                std::cout << std::endl;
                i.snapshot();
                std::cout << "[*] Analyzing " << i.getSnapshotsNum() << " snapshots for " << i.getFilename() << std::endl;
                result[i.getFilename()] = Utils::unifiedDiff(i.getSnapshots().first, i.getSnapshots().second);
            }
            std::ofstream outJson(Adam::outPath_);
            outJson << std::setw(4) << result << std::endl;
            outJson.close();
        }
        void run()
        {
            std::filesystem::current_path(std::filesystem::temp_directory_path() / "adam-tmp");
            std::cout << "[*] Writing tmp files to " << std::filesystem::current_path() << std::endl;
            for (auto i : this->dbPath_)
            {
                sqlite::SqliteWrapper wrapper(i);
                std::cout << "[*] Monitoring " << i << std::endl;
                wrapper.snapshot();
                wrappers_.push_back(wrapper);
            }
            std::signal(SIGINT, Adam::signalHandler);
            std::cout << "[*] Ready. Ctrl+C to stop." << std::endl;
            pause();
        }

    private:
        std::vector<std::string> dbPath_;
        inline static std::string outPath_;
        inline static std::vector<sqlite::SqliteWrapper> wrappers_;
        bool isLive_;
    };
}

#endif