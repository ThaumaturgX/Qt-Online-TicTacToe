#include "server.h"
#include <QDebug>
#include <iostream>
#include <algorithm>
Server::Server()
{
    listen(QHostAddress::Any, 2323);
    id = {1,0};
    turn = win = 0;
    qDebug() << "Started";
}

void Server::send()
{
    data.clear();

    for(auto &socket : sockets)
    {
        QDataStream out(&data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_3);
        out << board << int(socket->socketDescriptor() == sockets[id[turn]]->socketDescriptor()) << win;
        socket->write(data);
    }
}

void Server::newGame()
{
    turn = win = 0;
    board.clear();
    board.resize(3, QVector<char>(3, '.'));
    for(auto &[key, val] : rematch)
        val = 0;
    std::swap(id[0],id[1]);
}

int Server::checkWin() const
{
    // Check rows
    for(int i = 0; i < 3; i++)
    {
        if(board[i][0] == '.') continue;
        if(board[i][0] == board[i][1] && board[i][1] == board[i][2]) return 1;
    }

    // Check columns
    for(int i = 0; i < 3; i++)
    {
        if(board[0][i] == '.') continue;
        if(board[0][i] == board[1][i] && board[1][i] == board[2][i]) return 1;
    }

    // Check diagonals
    if(board[0][0] != '.' && board[0][0] == board[1][1] &&
            board[1][1] == board[2][2]) return 1;
    if(board[0][2] != '.' && board[0][2] == board[1][1] &&
            board[1][1] == board[2][0]) return 1;

    // Check if game is drawn
    int cnt = 0;
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            cnt += (board[i][j] == '.');
    if(!cnt) return 2;

    return 0;
}

void Server::deleteConnection()
{
    auto socket = dynamic_cast<QTcpSocket*>(sender());
    sockets.erase(std::find(sockets.begin(), sockets.end(), socket));
    qDebug() << "Disconnected: " << socket->socketDescriptor();
    socket->deleteLater();
}

void Server::incomingConnection(qintptr descriptor)
{
    auto socket = new QTcpSocket;
    socket->setSocketDescriptor(descriptor);

    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Server::deleteConnection);

    if(sockets.size() < 2)
    {
        sockets.push_back(socket);
        if(sockets.size() == 2)
        {
            newGame();
            send();
        }
        qDebug() << "Connected: " << descriptor;
    }
    else
        socket->disconnect();
}

void Server::slotReadyRead()
{
    auto socket = dynamic_cast<QTcpSocket*>(sender());
    auto desc = socket->socketDescriptor();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_3);
    if(in.status() == QDataStream::Ok)
    {
        QString action;
        in >> action;
        qDebug() << action << sockets.size();
        if(action == "place" && sockets.size() == 2)
        {
            int i, j;
            in >> i >> j;
            if(i >= 0 && i < 3 && j>=0 && j < 3 &&
                    board[i][j] == '.' && !win &&
                    desc == sockets[id[turn]]->socketDescriptor())
            {
                board[i][j] = (turn == 0) ? 'X' : 'O';
                turn = (turn + 1)%2;
                win = checkWin();
                send();
            }
        }
        else if(action == "rematch" && win)
        {
            int sum = 0;
            rematch[socket->socketDescriptor()] = 1;
            for(auto &[key, val] : rematch)
            {
                sum += val;
            }
            if(sum == 2)
            {
                newGame();
                send();
            }

        }
    }
    else
    {
        qDebug() << "Error reading";
    }

}


