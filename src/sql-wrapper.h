#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <vector>
#include <tuple>

#ifndef SQLITEPP
#define SQLITEPP

namespace sqlite
{
    typedef std::vector<std::string> snList;
    typedef std::pair<std::string, std::string> snapshots;
    class SqliteWrapper
    {
    public:
        SqliteWrapper(std::string filename)
        {
            this->filename_ = filename;
            int rc = sqlite3_open(filename.c_str(), &this->db_);
            if (rc != SQLITE_OK)
            {
                std::cout << "DB Error: " << sqlite3_errmsg(this->db_) << std::endl;
                sqlite3_close(this->db_);
                exit(1);
            }
        }
        /*
         * Take snapshot of entire database
         */
        void snapshot()
        {
            std::string snapName = this->getFilename();
            snapName = snapName + "-" + std::to_string(this->getSnapshotsNum()) + ".snap";
            dumpDb(snapName.c_str());
            this->sn_.push_back(snapName);
            return;
        }
        /*
         * Take snapshot of specified table
         */
        void snapshot(std::string table)
        {
            std::string snapName = this->getFilename();
            snapName = snapName + "-" + std::to_string(this->getSnapshotsNum()) + ".snap";
            dumpTable(snapName.c_str(), table.c_str());
            this->sn_.push_back(snapName);
            return;
        }
        std::string getFilename()
        {
            return std::filesystem::path(this->filename_).filename();
        }
        int getSnapshotsNum()
        {
            return this->sn_.size();
        }
        snapshots getSnapshots()
        {
            if (getSnapshotsNum() < 2)
                exit(1);
            return std::make_pair(this->sn_[0], this->sn_[1]);
        }
        virtual ~SqliteWrapper()
        {
            sqlite3_close(this->db_);
        }

    private:
        int dumpTable(const char *filename, const char *tablename)
        {
            FILE *fp = NULL;
            sqlite3_stmt *stmt_table = NULL;
            int ret = 0;
            int index = 0;
            int col_cnt = 0;
            char cmd[16000] = {0};
            char tmp_cmd[16000] = {0};
            const char *data = NULL;

            fp = fopen(filename, "w");
            if (!fp)
            {
                std::cout << "[!] Error opening file for writing" << std::endl;
                sqlite3_close(this->db_);
                exit(1);
            }

            sprintf(cmd, "SELECT * FROM %s;", tablename);
            ret = sqlite3_prepare_v2(this->db_, cmd, -1, &stmt_table, NULL);

            if (ret != SQLITE_OK)
            {
                std::cout << "[!] Error running query: " << ret << std::endl;
                sqlite3_close(this->db_);
                exit(1);
            }

            ret = sqlite3_step(stmt_table);
            while (ret == SQLITE_ROW)
            {
                sprintf(cmd, "INSERT INTO \"%s\" VALUES(", tablename);
                col_cnt = sqlite3_column_count(stmt_table);
                for (index = 0; index < col_cnt; index++)
                {
                    if (index)
                        strcat(cmd, ",");

                    /* @TODO Add support for BLOBs */
                    if (sqlite3_column_type(stmt_table, index) == SQLITE_BLOB)
                    {
                        sprintf(tmp_cmd, "{'%s':'blob'}", sqlite3_column_name(stmt_table, index));
                        strcat(cmd, tmp_cmd);
                    }
                    else
                    {
                        /* @TODO Check size of data before copying to cmd */
                        data = (const char *)sqlite3_column_text(stmt_table, index);
                        if (data)
                        {
                            if (sqlite3_column_type(stmt_table, index) == SQLITE_TEXT)
                            {
                                sprintf(tmp_cmd, "{'%s':'%s'}", sqlite3_column_name(stmt_table, index), data);
                                strcat(cmd, tmp_cmd);
                            }
                            else
                            {
                                sprintf(tmp_cmd, "{'%s':%s}", sqlite3_column_name(stmt_table, index), data);
                                strcat(cmd, tmp_cmd);
                            }
                        }
                        else
                        {
                            sprintf(tmp_cmd, "{'%s':NULL}", sqlite3_column_name(stmt_table, index));
                            strcat(cmd, tmp_cmd);
                        }
                    }
                }
                fprintf(fp, "%s);\n", cmd);
                ret = sqlite3_step(stmt_table);
            }

            if (stmt_table)
                sqlite3_finalize(stmt_table);
            if (fp)
            {
                fclose(fp);
            }

            return ret;
        }
        int dumpDb(const char *filename)
        {
            FILE *fp = NULL;
            sqlite3_stmt *stmt_table = NULL;
            sqlite3_stmt *stmt_data = NULL;
            const char *table_name = NULL;
            const char *data = NULL;
            int col_cnt = 0;
            int ret = 0;
            int index = 0;
            char cmd[16000] = {0};
            char tmp_cmd[16000] = {0};

            fp = fopen(filename, "w");

            if (!fp)
            {
                std::cout << "[!] Error opening file" << std::endl;
                sqlite3_close(this->db_);
                exit(1);
            }

            ret = sqlite3_prepare_v2(this->db_, "SELECT sql,tbl_name FROM sqlite_master WHERE type = 'table';",
                                     -1, &stmt_table, NULL);

            if (ret != SQLITE_OK)
            {
                std::cout << "[!] Error running query: " << ret << std::endl;
                sqlite3_close(this->db_);
                exit(1);
            }

            ret = sqlite3_step(stmt_table);
            while (ret == SQLITE_ROW)
            {
                table_name = (const char *)sqlite3_column_text(stmt_table, 1);
                if (!table_name)
                {
                    std::cout << "[!] Error running query: " << ret << std::endl;
                    sqlite3_close(this->db_);
                    exit(1);
                }

                /* fetch table data */
                sprintf(cmd, "SELECT * from %s;", table_name);

                ret = sqlite3_prepare_v2(this->db_, cmd, -1, &stmt_data, NULL);
                if (ret != SQLITE_OK)
                {
                    std::cout << "[!] Error running query: " << ret << std::endl;
                    sqlite3_close(this->db_);
                    exit(1);
                }

                ret = sqlite3_step(stmt_data);
                while (ret == SQLITE_ROW)
                {
                    sprintf(cmd, "INSERT INTO \"%s\" VALUES(", table_name);
                    col_cnt = sqlite3_column_count(stmt_data);
                    for (index = 0; index < col_cnt; index++)
                    {
                        if (index)
                            strcat(cmd, ",");

                        /* @TODO Add support for BLOBs */
                        if (sqlite3_column_type(stmt_data, index) == SQLITE_BLOB)
                        {
                            sprintf(tmp_cmd, "{'%s':'blob'}", sqlite3_column_name(stmt_data, index));
                            strcat(cmd, tmp_cmd);
                        }
                        else
                        {
                            /* @TODO Check size of data before copying to cmd */
                            data = (const char *)sqlite3_column_text(stmt_data, index);
                            if (data)
                            {
                                if (sqlite3_column_type(stmt_data, index) == SQLITE_TEXT)
                                {
                                    sprintf(tmp_cmd, "{'%s':'%s'}", sqlite3_column_name(stmt_data, index), data);
                                    strcat(cmd, tmp_cmd);
                                }
                                else
                                {
                                    sprintf(tmp_cmd, "{'%s':%s}", sqlite3_column_name(stmt_data, index), data);
                                    strcat(cmd, tmp_cmd);
                                }
                            }
                            else
                            {
                                sprintf(tmp_cmd, "{'%s':NULL}", sqlite3_column_name(stmt_data, index));
                                strcat(cmd, tmp_cmd);
                            }
                        }
                    }
                    fprintf(fp, "%s);\n", cmd);
                    ret = sqlite3_step(stmt_data);
                }
                if (stmt_data)
                    sqlite3_finalize(stmt_data);
                ret = sqlite3_step(stmt_table);
            }

            if (stmt_table)
                sqlite3_finalize(stmt_table);
            if (fp)
            {
                fclose(fp);
            }
            return ret;
        }
        std::string filename_;
        sqlite3 *db_;
        snList sn_;
    };
}

#endif