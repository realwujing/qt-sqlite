#pragma once

#include <QtSql>
#include <QString>
#include <QMutex>
#include <QMutexLocker>

#include "singleton.h"

class Connection : public QObject
{
    Q_OBJECT

public:
    explicit Connection(QObject *parent = nullptr);
    ~Connection();
    QSqlQuery execute(QString &sql);   // 执行sql语句
    
private:
    QSqlDatabase getConnection();                   // 创建数据库连接
    void closeConnection(QSqlDatabase &connection); // 关闭连接

private:
    
    QString databaseName; // 如果是 SQLite 则为数据库文件名
    QString databaseType;

    bool testOnBorrow;       // 取得连接的时候验证连接是否有效
    QString testOnBorrowSql; // 测试访问数据库的 SQL

    QSqlDatabase connection;

    static QMutex mutex;
};