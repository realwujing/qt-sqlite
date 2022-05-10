#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtConcurrent/QtConcurrent>

#include "connection_pool.h"

int test()
{
    QString sql = "select 1";

    QSqlQuery sqlQuery(sql, *(CONNECTION_POOL->getConnection().data()));
    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    for (size_t i = 0; i < 10; i++)
    {
        QtConcurrent::run([=]()
                      { int ret = test();
                        // qInfo() << "ret:" << ret;
                        });
    }

    return a.exec();
}
