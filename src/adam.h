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

#include <chrono>
#include <thread>
#include <unistd.h>
#include <csignal>
#include <memory>
#include "sql-wrapper.h"
#include "common.h"
#include "spinners.hpp"

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
            this->spin_ = new spinners::Spinner();
            this->spin_->setInterval(100);
            this->spin_->setSymbols("dots");
            std::filesystem::create_directory(std::filesystem::temp_directory_path() / "adam-tmp");
            std::cout << std::endl << "[*] " << ADAM_VERSION_STR << std::endl;
        }
        virtual ~Adam()
        {
            std::filesystem::remove_all(std::filesystem::temp_directory_path() / "adam-tmp");
            delete this->spin_;
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
            for (auto i : this->dbPath_)
            {
                auto wrapper = std::make_shared<sqlite::SqliteWrapper>(i);
                this->spin_->start("Snapshotting " + i, true);
                if (this->isTableMode_)
                    wrapper->snapshot(this->tableName_);
                else
                    wrapper->snapshot();
                this->wrappers_.push_back(wrapper);
                this->spin_->stop();
            }
            std::signal(SIGINT, Adam::signalHandler);
            std::cout << "[*] Ready. Ctrl+C to stop." << std::endl;

            pause();

            json result;
            std::cout << std::endl;
            for (auto i : this->wrappers_)
            {
                this->spin_->start("Processing " + i->getFilename(), true);
                if (this->isTableMode_)
                    i->snapshot(this->tableName_);
                else
                    i->snapshot();
                result[i->getFilename()] = Utils::unifiedDiff(i->getSnapshots().first, i->getSnapshots().second);
                this->spin_->stop();
            }
            std::cout << "[*] Writing output to " << this->outPath_ << std::endl;
            std::ofstream outJson(this->outPath_);
            outJson << std::setw(4) << result << std::endl;
            outJson.close();
            std::cout << "[*] Finished ;-)" << std::endl << std::endl;
        }

    private:
        std::vector<std::string> dbPath_;
        std::string outPath_;
        std::vector<std::shared_ptr<sqlite::SqliteWrapper>> wrappers_;
        std::string tableName_;
        bool isLive_;
        bool isTableMode_;
        spinners::Spinner *spin_;
    };
}

#endif