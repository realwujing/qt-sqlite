#include "connection_pool.h"

#include <QDebug>

#define HOST_NAME "127.0.0.1"
#define DATABASE_PATH_PREFIX "/deepin/linglong/layers/"
#define DATABASE_NAME "InstalledAppInfo.db"
#define DATABASE_TYPE "QSQLITE"
#define TEST_ON_BORROW_SQL "SELECT 1"
#define MAX_WAIT_TIME 1000
#define WAIT_INTERVAL 200
#define MAX_CONNECTION_COUNT 5

QMutex ConnectionPool::mutex;
QWaitCondition ConnectionPool::waitConnection;
int ConnectionPool::deleteCount = 0;

ConnectionPool::ConnectionPool(QObject *parent)
    : QObject(parent)
    , hostName(HOST_NAME)
    , databaseName(QString(DATABASE_PATH_PREFIX) + QString(DATABASE_NAME))
    , databaseType(DATABASE_TYPE)
    , testOnBorrow(true)
    , testOnBorrowSql(TEST_ON_BORROW_SQL)
    , maxWaitTime(MAX_WAIT_TIME)
    , waitInterval(WAIT_INTERVAL)
    , maxConnectionCount(MAX_CONNECTION_COUNT)
{
    // qDebug() << "ConnectionPool::ConnectionPool";
}

ConnectionPool::~ConnectionPool()
{
    foreach (QString connectionName, usedConnectionNames)
    {
        QSqlDatabase::removeDatabase(connectionName);
    }

    foreach (QString connectionName, unusedConnectionNames)
    {
        QSqlDatabase::removeDatabase(connectionName);
    }
}

QSharedPointer<QSqlDatabase> ConnectionPool::getConnection()
{
    QMutexLocker locker(&mutex);    // 加锁，加锁不成功则阻塞

    QString connectionName;

    // 已创建连接数
    int connectionCount = unusedConnectionNames.size() + usedConnectionNames.size();
    bool waitRet = false;
    // 如果连接已经用完，等待 waitInterval 毫秒看看是否有可用连接，最长等待 maxWaitTime 毫秒
    for (int i = 0; (i < maxWaitTime) && (unusedConnectionNames.size() == 0) && (connectionCount == maxConnectionCount);
         i += waitInterval)
    {
        waitConnection.wait(&mutex, waitInterval);  // 先解锁，线程等待一定的时间，如果超时或有信号触发，线程唤醒，唤醒后重新加锁
        // 重新计算已创建连接数
        connectionCount = unusedConnectionNames.size() + usedConnectionNames.size();
    }

    if (0 < unusedConnectionNames.size())
    {
        // 有已经回收的连接，复用它们
        connectionName = unusedConnectionNames.dequeue();
    }
    else if (connectionCount < maxConnectionCount)
    {
        // 没有已经回收的连接，但是没有达到最大连接数，则创建新的连接
        connectionName = QString("UserConnection - %1").arg(connectionCount + 1);
    }
    else
    {
        qWarning() << "connectionCount = " << connectionCount << "Cannot create more connections.";
        return QSharedPointer<QSqlDatabase>(new QSqlDatabase, closeConnection);
    }

    // 创建连接
    static QSqlDatabase sqlDatabase = createConnection(connectionName);
    QSharedPointer<QSqlDatabase> connection(&sqlDatabase, closeConnection);

    // 有效的连接才放入 usedConnectionNames
    if (connection->isOpen())
    {
        usedConnectionNames.enqueue(connectionName);
    }
    else
    {
        // 连接无效
        qWarning() << "connections is open fail.";
        return QSharedPointer<QSqlDatabase>(new QSqlDatabase, closeConnection);
    }
    return connection;
}

void ConnectionPool::closeConnection(QSqlDatabase *connection)
{
    QMutexLocker locker(&mutex);

    ConnectionPool *pool = ConnectionPool::instance();
    QString connectionName = connection->connectionName();

    // 如果是我们创建的连接，从 used 里删除，放入 unused 里
    if (pool->usedConnectionNames.contains(connectionName))
    {
        pool->usedConnectionNames.removeOne(connectionName);
        pool->unusedConnectionNames.enqueue(connectionName);
        deleteCount++;
        qInfo() << "currentThreadId:" << QThread::currentThreadId() << "deleteCount:" << deleteCount << "connectionName:" << connectionName << "total.size:" << pool->usedConnectionNames.size() + pool->unusedConnectionNames.size() << "used.size:" << pool->usedConnectionNames.size() << "unused.size:" << pool->unusedConnectionNames.size();
        waitConnection.wakeOne();
    }
}

QSqlDatabase ConnectionPool::createConnection(const QString &connectionName)
{
    // 连接已经创建过了，复用它，而不是重新创建
    if (QSqlDatabase::contains(connectionName))
    {
        QSqlDatabase multiplexedConnection = QSqlDatabase::database(connectionName);

        if (!multiplexedConnection.isOpen())
        {
            // 返回连接前访问数据库，如果连接断开，重新建立连接
            qDebug() << "Reconnection: " << testOnBorrowSql << " for " << connectionName;
            QSqlQuery query(testOnBorrowSql, multiplexedConnection);

            if (!multiplexedConnection.open() && QSqlError::NoError != query.lastError().type())
            {
                qDebug() << "Open datatabase error:" << multiplexedConnection.lastError().text();
                return QSqlDatabase();
            }
        }

        return multiplexedConnection;
    }

    // 创建一个新的连接
    QSqlDatabase connection = QSqlDatabase::addDatabase(databaseType, connectionName);
    // connection.setHostName(hostName);
    connection.setDatabaseName(databaseName);

    if (!connection.open())
    {
        qDebug() << "Open datatabase error:" << connection.lastError().text();
        return QSqlDatabase();
    }

    return connection;
}
