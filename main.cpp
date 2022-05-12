#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtConcurrent/QtConcurrent>

#include "connection.h"

int test()
{
    QString sql = "select 1";
    Connection connection;
    QSqlQuery query = connection.execute(sql);
    while (query.next()) {
        QString sqlResult = query.value(0).toString();
        qDebug() << "select 1:" << sqlResult;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    for (size_t i = 0; i < 1000; i++)
    {
        QtConcurrent::run([=]()
                      { int ret = test();
                        // qInfo() << "ret:" << ret;
                        });
    }

    return a.exec();
}
