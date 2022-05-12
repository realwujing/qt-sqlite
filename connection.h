#pragma once

#include <QtSql>
#include <QQueue>
#include <QString>
#include <QMutex>
#include <QMutexLocker>

#include "singleton.h"

class Connection : public QObject, public Singleton<Connection>
{
    Q_OBJECT
    friend class Singleton<Connection>;

public:
    QSqlDatabase getConnection();                   // 创建数据库连接
    void closeConnection(QSqlDatabase &connection); // 关闭连接
private:
    explicit Connection(QObject *parent = nullptr);

private:
    
    QString databaseName; // 如果是 SQLite 则为数据库文件名
    QString databaseType;

    bool testOnBorrow;       // 取得连接的时候验证连接是否有效
    QString testOnBorrowSql; // 测试访问数据库的 SQL

    static QMutex mutex;
    static QWaitCondition waitConnection;
};

#define CONNECTION Connection::instance()
