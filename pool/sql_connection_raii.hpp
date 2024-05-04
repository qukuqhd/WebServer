/**
 * @file sql_connection_rall.hpp
 * @author {gangx} ({gangx6906@gmail.com})
 * @brief 
 * @version 0.1
 * @date 2024-05-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#ifndef _SQL_CONN_RAII_H_
#define _SQL_CONN_RAII_H_
#include "sql_connection_pool.hpp"
class SqlConnRAII{
    public:
        SqlConnRAII(MYSQL**sql,SqlConnPool*conn_pool){
            assert(conn_pool);
            *sql = conn_pool->GetConn();
            sql_ = *sql;
            conn_pool_ = conn_pool;
        }
        ~SqlConnRAII(){
            if(sql_) conn_pool_->FreeConn(sql_);
        }
    private:
        MYSQL* sql_;
        SqlConnPool *conn_pool_;
};
#endif