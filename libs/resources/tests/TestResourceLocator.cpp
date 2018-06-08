/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TestResourceLocator.h"

#include <QTest>
#include <QVersionNumber>
#include <QDirIterator>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KisResourceLocator.h>
#include <KritaVersionWrapper.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif

void TestResourceLocator::initTestCase()
{
    srcLocation = QString(FILES_DATA_DIR);
    QVERIFY2(QDir(srcLocation).exists(), srcLocation.toUtf8());
    dstLocation = QString(FILES_DEST_DIR);
    cleanDstLocation();
    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    cfg.writeEntry(KisResourceLocator::resourceLocationKey, dstLocation);
}

void TestResourceLocator::testLocator()
{
    KisResourceLocator locator;
    KisResourceLocator::LocatorError r = locator.initialize(srcLocation);
    if (!locator.errorMessages().isEmpty()) qDebug() << locator.errorMessages();
    QVERIFY(r == KisResourceLocator::LocatorError::Ok);
    QVERIFY(QDir(dstLocation).exists());
    Q_FOREACH(const QString &folder, KisResourceLocator::resourceTypeFolders) {
        QDir dstDir(dstLocation + '/' + folder + '/');
        QDir srcDir(srcLocation + '/' + folder + '/');

        QVERIFY(dstDir.exists());
        QVERIFY(dstDir.entryList(QDir::Files | QDir::NoDotAndDotDot) == srcDir.entryList(QDir::Files | QDir::NoDotAndDotDot));
    }

    QFile f(dstLocation + '/' + "KRITA_RESOURCE_VERSION");
    QVERIFY(f.exists());
    f.open(QFile::ReadOnly);
    QVersionNumber version = QVersionNumber::fromString(QString::fromUtf8(f.readAll()));
    QVERIFY(version == QVersionNumber::fromString(KritaVersionWrapper::versionString()));

    locator.synchronizeDb();

}

void TestResourceLocator::cleanupTestCase()
{
    cleanDstLocation();
}

bool TestResourceLocator::cleanDstLocation()
{
    if (QDir(dstLocation).exists()) {
        {
            QDirIterator iter(dstLocation, QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
            while (iter.hasNext()) {
                iter.next();
                QFile f(iter.filePath());
                bool r = f.remove();
                //qDebug() << (r ? "Removed" : "Failed to remove") << iter.filePath();
            }
        }
        {
            QDirIterator iter(dstLocation, QStringList() << "*", QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (iter.hasNext()) {
                iter.next();
                bool r = QDir(iter.filePath()).rmpath(iter.filePath());
                //qDebug() << (r ? "Removed" : "Failed to remove") << iter.filePath();
            }
        }

        return QDir().rmpath(dstLocation);
    }
    return true;
}

QTEST_MAIN(TestResourceLocator)

