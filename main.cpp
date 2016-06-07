// Copyright (c) 2014-2015, The Monero Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QStandardPaths>
#include "clipboardAdapter.h"
#include "filter.h"
#include "oscursor.h"
#include "WalletManager.h"
#include "Wallet.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    filter *eventFilter = new filter;
    app.installEventFilter(eventFilter);

    qmlRegisterType<clipboardAdapter>("moneroComponents", 1, 0, "Clipboard");
    qmlRegisterInterface<Wallet>("Wallet");


    QQmlApplicationEngine engine;

    OSCursor cursor;
    engine.rootContext()->setContextProperty("globalCursor", &cursor);
    engine.rootContext()->setContextProperty("walletManager", WalletManager::instance());

//  export to QML monero accounts root directory
//  wizard is talking about where
//  to save the wallet file (.keys, .bin), they have to be user-accessible for
//  backups - I reckon we save that in My Documents\Monero Accounts\ on
//  Windows, ~/Monero Accounts/ on nix / osx

#ifdef Q_OS_WIN
    QStringList moneroAccountsRootDir = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
#elif defined(Q_OS_UNIX)
    QStringList moneroAccountsRootDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
#endif

    if (!moneroAccountsRootDir.empty()) {
        QString moneroAccountsDir = moneroAccountsRootDir.at(0) + "/Monero Accounts";
        QDir tempDir;
        tempDir.mkpath(moneroAccountsDir);
        engine.rootContext()->setContextProperty("moneroAccountsDir", moneroAccountsDir);
    }

    engine.rootContext()->setContextProperty("applicationDirectory", QApplication::applicationDirPath());
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));
    QObject *rootObject = engine.rootObjects().first();

    QObject::connect(eventFilter, SIGNAL(sequencePressed(QVariant,QVariant)), rootObject, SLOT(sequencePressed(QVariant,QVariant)));
    QObject::connect(eventFilter, SIGNAL(sequenceReleased(QVariant,QVariant)), rootObject, SLOT(sequenceReleased(QVariant,QVariant)));
    QObject::connect(eventFilter, SIGNAL(mousePressed(QVariant,QVariant,QVariant)), rootObject, SLOT(mousePressed(QVariant,QVariant,QVariant)));
    QObject::connect(eventFilter, SIGNAL(mouseReleased(QVariant,QVariant,QVariant)), rootObject, SLOT(mouseReleased(QVariant,QVariant,QVariant)));

    return app.exec();
}
