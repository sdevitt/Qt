/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QStringList>
#include <QtTest>

#include <sstream>
#include <string>
#include <vector>

class tst_QStringList: public QObject
{
    Q_OBJECT

private slots:
    void join() const;
    void join_data() const;

    void split_qlist_qbytearray() const;
    void split_qlist_qbytearray_data() const { return split_data(); }

    void split_data() const;
    void split_qlist_qstring() const;
    void split_qlist_qstring_data() const { return split_data(); }

    void split_stdvector_stdstring() const;
    void split_stdvector_stdstring_data() const { return split_data(); }

    void split_stdvector_stdwstring() const;
    void split_stdvector_stdwstring_data() const { return split_data(); }

    void split_stdlist_stdstring() const;
    void split_stdlist_stdstring_data() const { return split_data(); }

private:
    static QStringList populateList(const int count, const QString &unit);
    static QString populateString(const int count, const QString &unit);
};

QStringList tst_QStringList::populateList(const int count, const QString &unit)
{
    QStringList retval;

    for (int i = 0; i < count; ++i)
        retval.append(unit);

    return retval;
}

QString tst_QStringList::populateString(const int count, const QString &unit)
{
    QString retval;

    for (int i = 0; i < count; ++i) {
        retval.append(unit);
        retval.append(QLatin1Char(':'));
    }

    return retval;
}

void tst_QStringList::join() const
{
    QFETCH(QStringList, input);
    QFETCH(QString, separator);

    QBENCHMARK {
        input.join(separator);
    }
}

void tst_QStringList::join_data() const
{
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<QString>("separator");

    QTest::newRow("")
        << populateList(100, QLatin1String("unit"))
        << QString();

    QTest::newRow("")
        << populateList(1000, QLatin1String("unit"))
        << QString();

    QTest::newRow("")
        << populateList(10000, QLatin1String("unit"))
        << QString();

    QTest::newRow("")
        << populateList(100000, QLatin1String("unit"))
        << QString();
}

void tst_QStringList::split_data() const
{
    QTest::addColumn<QString>("input");
    QString unit = QLatin1String("unit") + QString(100, QLatin1Char('s'));
    //QTest::newRow("") << populateString(10, unit);
    QTest::newRow("") << populateString(100, unit);
    //QTest::newRow("") << populateString(100, unit);
    //QTest::newRow("") << populateString(1000, unit);
    //QTest::newRow("") << populateString(10000, unit);
}

void tst_QStringList::split_qlist_qbytearray() const
{
    QFETCH(QString, input);
    const char splitChar = ':';
    QByteArray ba = input.toLatin1();

    QBENCHMARK {
        ba.split(splitChar);
    }
}

void tst_QStringList::split_qlist_qstring() const
{
    QFETCH(QString, input);
    const QChar splitChar = ':';

    QBENCHMARK {
        input.split(splitChar);
    }
}

void tst_QStringList::split_stdvector_stdstring() const
{
#ifndef QT_NO_STL
    QFETCH(QString, input);
    const char split_char = ':';
    std::string stdinput = input.toStdString();

    QBENCHMARK {
        std::istringstream split(stdinput);
        std::vector<std::string> token;
        for (std::string each;
             std::getline(split, each, split_char);
             token.push_back(each))
            ;
    }
#endif
}

void tst_QStringList::split_stdvector_stdwstring() const
{
#ifndef QT_NO_STL
    QFETCH(QString, input);
    const wchar_t split_char = ':';
    std::wstring stdinput = input.toStdWString();

    QBENCHMARK {
        std::wistringstream split(stdinput);
        std::vector<std::wstring> token;
        for (std::wstring each;
             std::getline(split, each, split_char);
             token.push_back(each))
            ;
    }
#endif
}

void tst_QStringList::split_stdlist_stdstring() const
{
    QFETCH(QString, input);
    const char split_char = ':';
    std::string stdinput = input.toStdString();

    QBENCHMARK {
        std::istringstream split(stdinput);
        std::list<std::string> token;
        for (std::string each;
             std::getline(split, each, split_char);
             token.push_back(each))
            ;
    }
}

QTEST_MAIN(tst_QStringList)

#include "main.moc"
