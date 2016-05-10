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
#include <QDebug>
#include <QString>
#include <QThread>
#include <QFile>
#include <QUrl>
#include "remotepicberry.h"

enum srv_command : char{
    SRV_PB_VER      = '0',
    SRV_RESET       = '1',
    SRV_ENTER       = '2',
    SRV_EXIT        = '3',
    SRV_DEV_ID      = '4',
    SRV_ERASE       = '5',
    SRV_READ        = '6',
    SRV_WRITE       = '7',
    SRV_BLANKCHECK  = '8',
    SRV_REGDUMP     = '9',
    SRV_SET_FAMILY  = 'A'
};

RemotePicberry::RemotePicberry(QObject *parent) : QThread(parent)
{
    working = false;
    mode = SRV_READ;
}

void RemotePicberry::run()
{
    char response[6];
    int percentage = 0;
    bool run_twice = false;

    if(mode == SRV_WRITE){
        run_twice = true;
        sendFile();
        sendFinishFileSend();
    }

    while(working){
        if(picberry_socket.bytesAvailable() ||
           picberry_socket.waitForReadyRead()){
            picberry_socket.readLine(response, 5);
            qDebug() << response;
            if(QString(response) == "@FIN" && !run_twice){
                working = false;
            }
            else if(QString(response) == "@ERR"){
                sendError();
                working = false;
                return;
            }
            else{
                sscanf(response, "@%d", &percentage);
                sendProgress(percentage);
                if(QString(response) == "@100" && run_twice){
                    sendFinishFirstRun();
                    run_twice = false;
                }
            }
        }
    }
    if(mode == SRV_READ){
        sendStartFileReceive();
        receiveFile();
    }
    picberry_socket.readAll();
    sendFinish();
}

bool RemotePicberry::connectSocket(const QString &address, int port) {
    qDebug() << "Trying to connect to host " << address << ":" << port;
    picberry_socket.connectToHost(address, port);
    if (picberry_socket.waitForConnected(1000)){
        qDebug() << "Connected!";
        picberry_socket.readAll();
        return true;
    }
    else{
        qDebug() << "Connection timeout.";
        return false;
    }
}

void RemotePicberry::disconnectSocket() {
    picberry_socket.disconnectFromHost();
}

void RemotePicberry::enterProgramMode() {
    picberry_socket.putChar(SRV_ENTER);
    picberry_socket.write("\r\n");
    picberry_socket.flush();
}

void RemotePicberry::exitProgramMode() {
    picberry_socket.putChar(SRV_EXIT);
    picberry_socket.write("\r\n");
    picberry_socket.flush();
}

QString RemotePicberry::getPicberryVersion() {
    char response[10];
    picberry_socket.putChar(SRV_PB_VER);
    picberry_socket.write("\r\n");
    picberry_socket.flush();
    if(picberry_socket.waitForReadyRead(3000)){
        picberry_socket.readLine(response, 9);
        qDebug() << "getPicberryVersion: " << response;
        return QString(response);
    }
    else{
        qDebug() << "getPicberryVersion() TIMEOUT";
        return QString("NC");
    }
}

QString RemotePicberry::setFamily(const QString &family) {
    char response[10];
    picberry_socket.putChar(SRV_SET_FAMILY);
    QByteArray ba = family.toLatin1();
    picberry_socket.write(ba.data());
    picberry_socket.write("\r\n");
    picberry_socket.flush();
    if(picberry_socket.waitForReadyRead(3000)){
        picberry_socket.readLine(response, 10);
        qDebug() << "setFamily: " << response;
        return QString(response);
    }
    else{
        qDebug() << "setFamily() TIMEOUT";
        return QString("NC");
    }
}

QString RemotePicberry::getDeviceID() {
    char response[75];
    picberry_socket.putChar(SRV_DEV_ID);
    picberry_socket.write("\r\n");
    picberry_socket.flush();
    if(picberry_socket.waitForReadyRead(3000)){
        picberry_socket.readLine(response, 75);
        qDebug() << "getDeviceID: " << response;
        return QString(response);
    }
    else{
        qDebug() << "getDeviceID() TIMEOUT";
        return QString("NC");
    }
}

void RemotePicberry::eraseMemory() {
    picberry_socket.putChar(SRV_ERASE);
    picberry_socket.write("\r\n");
    picberry_socket.flush();
    mode = SRV_ERASE;
    working = true;
    start();
}

void RemotePicberry::readMemory(QString filename) {
    picberry_socket.putChar(SRV_READ);
    picberry_socket.write("\r\n");
    picberry_socket.flush();
    mode = SRV_READ;
    working = true;
    target_filename = filename;
    start();
}

void RemotePicberry::writeMemory(QString filename) {
    picberry_socket.putChar(SRV_WRITE);
    picberry_socket.write("\r\n");
    picberry_socket.flush();
    mode = SRV_WRITE;
    working = true;
    target_filename = filename;
    start();
}

void RemotePicberry::receiveFile(){
    char response[45];
    QUrl url(target_filename);
    QFile file(url.toLocalFile());
    file.open(QIODevice::ReadWrite);
    QTextStream stream(&file);
    while(true){
        if(picberry_socket.bytesAvailable() ||
           picberry_socket.waitForReadyRead()){
            picberry_socket.readLine(response, 44);
            if (QString(response) == "@FIN")
                break;
            else
                stream << response;
        }
    }
}

void RemotePicberry::sendFile(){
    QString line;
    QUrl url(target_filename);
    QFile file(url.toLocalFile());
    file.open(QIODevice::ReadWrite);
    QTextStream stream(&file);
    while(!stream.atEnd()){
        line = stream.readLine();
        QByteArray ba = line.toUtf8();
        picberry_socket.write(ba.data());
        picberry_socket.putChar('\n');
        picberry_socket.flush();
    }
    picberry_socket.putChar('@');
    picberry_socket.flush();
}
