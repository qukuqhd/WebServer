#include "sql_connection_pool.hpp"
SqlConnPool::SqlConnPool(){
    useCount_ = 0;
    freeCount_ = 0;
}
SqlConnPool* SqlConnPool::Instance(){
    static SqlConnPool pool;
    return &pool;
}

void SqlConnPool::Init(const char *host, int port, const char *user, const char *pwd, const char *db, int conn_size)
{
    assert(conn_size>0);
    for(int i = 0;i < conn_size;i++){
        MYSQL * sql = nullptr;
        sql = mysql_init(sql);
        if(!sql){
            LOG_ERROR("Mysql init error!");
            assert(sql);
        }
        sql = mysql_real_connect(sql,host,user,pwd,db,port,nullptr,0);
        if(!sql){
            LOG_ERROR("Mysql connect error!");
        }
        connQue_.push(sql);
    }
    MAX_CONN_ = conn_size;
    sem_init(&semId_,0,MAX_CONN_);
}
MYSQL* SqlConnPool::GetConn(){
    MYSQL* sql = nullptr;
    if(connQue_.empty()){
        LOG_WARN("SQlConnPool Busy!");
        return nullptr;
    }
    sem_wait(&semId_);
    {
        std::lock_guard<std::mutex> locker(mtx);
        sql = connQue_.front();
        connQue_.pop();
    }
    return sql;
}

void SqlConnPool::FreeConn (MYSQL*sql){
    assert(sql);
    std::lock_guard<std::mutex> locker(mtx);
    connQue_.push(sql);
    sem_post(&semId_);
}

void SqlConnPool::ClosePool(){
    std::lock_guard<std::mutex> locker(mtx);
    while (!connQue_.empty())
    {   
        auto item = connQue_.front();
        connQue_.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

int SqlConnPool::GetFreeConnCount(){
    std::lock_guard<std::mutex> locker(mtx);
    return connQue_.size();
}
SqlConnPool::~SqlConnPool (){
    ClosePool();
}