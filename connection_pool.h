#pragma once

#include <QtSql>
#include <QQueue>
#include <QString>
#include <QMutex>
#include <QMutexLocker>

#include "singleton.h"

class ConnectionPool : public QObject, public Singleton<ConnectionPool>
{
    Q_OBJECT
    friend class Singleton<ConnectionPool>;
public:
    
    QSharedPointer<QSqlDatabase> getConnection(); //用于从连接池里获取连接
    QSqlDatabase createConnection(const QString &connectionName); // 创建数据库连接
    static void closeConnection(QSqlDatabase *connection); //并不会真正的关闭连接，而是把连接放回连接池复用

private:
    explicit ConnectionPool(QObject *parent = nullptr);
    ~ConnectionPool();
    
private:

    QQueue<QString> usedConnectionNames;   // 已使用的数据库连接名
    QQueue<QString> unusedConnectionNames; // 未使用的数据库连接名

    // 数据库信息
    QString hostName;
    QString databaseName; // 如果是 SQLite 则为数据库文件名
    QString databaseType;

    bool testOnBorrow;       // 取得连接的时候验证连接是否有效
    QString testOnBorrowSql; // 测试访问数据库的 SQL

    int maxWaitTime;        // 获取连接最大等待时间
    int waitInterval;       // 尝试获取连接时等待间隔时间
    int maxConnectionCount; // 最大连接数

    static QMutex mutex;
    static QWaitCondition waitConnection;

    static int deleteCount;
};

#define CONNECTION_POOL ConnectionPool::instance()
