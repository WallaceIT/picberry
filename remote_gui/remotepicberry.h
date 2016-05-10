/*
 * Raspberry Pi PIC Programmer using GPIO connector
 * https://github.com/WallaceIT/picberry
 * Copyright 2016 Francesco Valla
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef REMOTEPICBERRY_H
#define REMOTEPICBERRY_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QTcpSocket>

class RemotePicberry : public QThread
{
    Q_OBJECT
public:
    explicit RemotePicberry(QObject *parent = 0);
    void run();

    Q_INVOKABLE bool connectSocket(const QString &address, int port);
    Q_INVOKABLE void disconnectSocket();
    Q_INVOKABLE void enterProgramMode();
    Q_INVOKABLE void exitProgramMode();
    Q_INVOKABLE QString getPicberryVersion();
    Q_INVOKABLE QString getDeviceID();
    Q_INVOKABLE QString setFamily(const QString &family);

signals:
    void sendProgress(int percentage);
    void sendFinish();
    void sendFinishFirstRun();
    void sendFinishFileSend();
    void sendStartFileReceive();
    void sendError();

public slots:
    void eraseMemory();
    void readMemory(QString filename);
    void writeMemory(QString filename);
private:
    QTcpSocket picberry_socket;
    bool working;
    char mode;
    void receiveFile();
    void sendFile();
    QString target_filename;
};

#endif // REMOTEPICBERRY_H
