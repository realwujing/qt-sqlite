#include "connection.h"

#include <QDebug>

#define DATABASE_PATH_PREFIX "/deepin/linglong/layers/"
#define DATABASE_NAME "InstalledAppInfo.db"
#define DATABASE_TYPE "QSQLITE"
#define TEST_ON_BORROW_SQL "SELECT 1"

QMutex Connection::mutex;
QWaitCondition Connection::waitConnection;

Connection::Connection(QObject *parent)
    : QObject(parent)
    , databaseName(QString(DATABASE_PATH_PREFIX) + QString(DATABASE_NAME))
    , databaseType(DATABASE_TYPE)
    , testOnBorrow(true)
    , testOnBorrowSql(TEST_ON_BORROW_SQL)
    
{
}

QSqlDatabase Connection::getConnection()
{
    QMutexLocker locker(&mutex);    // 加锁，加锁不成功则阻塞

    QString connectionName = QStringLiteral("connection_%1").arg(qintptr(QThread::currentThreadId()), 0, 16);

    // 创建一个新的连接
    QSqlDatabase connection = QSqlDatabase::addDatabase(databaseType, connectionName);
    // 设置sqlite数据库路径
    connection.setDatabaseName(databaseName);
    if (!connection.open())
    {
        qDebug() << "open database failed:" << connection.lastError().text();
    }
    else
    {
        QSqlQuery query(testOnBorrowSql, connection);
        if (QSqlError::NoError != query.lastError().type())
        {
            qDebug() << "Open datatabase error:" << connection.lastError().text();
        }
    }
    
    return connection;
}

void Connection::closeConnection(QSqlDatabase &connection)
{
    QMutexLocker locker(&mutex);

    QString connectionName = connection.connectionName();
    connection.close();
    connection = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
    qDebug() << "connectionNames:" << QSqlDatabase::connectionNames();
    
}
