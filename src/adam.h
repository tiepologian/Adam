#include <chrono>
#include <thread>
#include <unistd.h>
#include <csignal>
#include "sql-wrapper.h"
#include "common.h"
#include <memory>

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
            this->isTableMode_ = false;
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
            this->outPath_ = s;
            std::cout << "[*] Writing output to " << s << std::endl;
        }
        void setTableMode(std::string table)
        {
            this->isTableMode_ = true;
            this->tableName_ = table;
        }
        static void signalHandler(int signal __attribute__((unused)))
        {
            return;
        }
        void run()
        {
            std::filesystem::current_path(std::filesystem::temp_directory_path() / "adam-tmp");
            std::cout << "[*] Writing tmp files to " << std::filesystem::current_path() << std::endl;
            for (auto i : this->dbPath_)
            {
                auto wrapper = std::make_shared<sqlite::SqliteWrapper>(i);
                std::cout << "[*] Monitoring " << i << std::endl;
                if (this->isTableMode_)
                    wrapper->snapshot(this->tableName_);
                else
                    wrapper->snapshot();
                this->wrappers_.push_back(wrapper);
            }
            std::signal(SIGINT, Adam::signalHandler);
            std::cout << "[*] Ready. Ctrl+C to stop." << std::endl;

            pause();

            json result;
            for (auto i : this->wrappers_)
            {
                if (this->isTableMode_)
                    i->snapshot(this->tableName_);
                else
                    i->snapshot();
                std::cout << std::endl << "[*] Analyzing " << i->getSnapshotsNum() << " snapshots for " << i->getFilename() << std::endl;
                result[i->getFilename()] = Utils::unifiedDiff(i->getSnapshots().first, i->getSnapshots().second);
            }
            std::ofstream outJson(this->outPath_);
            outJson << std::setw(4) << result << std::endl;
            outJson.close();
        }

    private:
        std::vector<std::string> dbPath_;
        std::string outPath_;
        std::vector<std::shared_ptr<sqlite::SqliteWrapper>> wrappers_;
        std::string tableName_;
        bool isLive_;
        bool isTableMode_;
    };
}

#endif