#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <unordered_map>

class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server();
private:
    QVector<QTcpSocket*> sockets;
    QVector<int> id;
    std::unordered_map<int, int> rematch;
    QByteArray data;
    void send();
    void newGame();
    int checkWin() const;
    void deleteConnection();
    QVector<QVector<char>> board;
    int turn;
    int win;

public slots:
    void incomingConnection(qintptr descriptor);
    void slotReadyRead();
};

#endif // SERVER_H
