/**
 * @file sql_connection_pool.hpp
 * @author {gangx} ({gangx6906@gmail.com})
 * @brief
 * @version 0.1
 * @date 2024-05-04
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#ifndef _SQL_CONN_POOL_H_
#define _SQL_CONN_POOL_H_
#include "../log/log.hpp"
#include <mysql/mysql.h>
#include <queue>
#include <semaphore.h>
class SqlConnPool
{
public:
    static SqlConnPool *Instance();
    MYSQL *GetConn();
    void FreeConn(MYSQL *sql);
    int GetFreeConnCount();

    void Init(const char *host, int port, const char *user, const char *pwd, const char *db, int conn_size);
    void ClosePool();
private:
    SqlConnPool();
    ~SqlConnPool();
    SqlConnPool &operator=(const SqlConnPool &other) = delete;
    SqlConnPool(const SqlConnPool &other) = delete;
    int MAX_CONN_;
    int useCount_;
    int freeCount_;

    std::queue<MYSQL *> connQue_;
    std::mutex mtx;
    sem_t semId_;
};
#endif