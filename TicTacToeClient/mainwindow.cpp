#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Tic Tac Toe Client");
    setFixedSize(size());
    createBoard();
    ui->rematchButton->hide();

    socket = new QTcpSocket;
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

MainWindow::~MainWindow()
{
    socket->disconnectFromHost();
    delete ui;
}

void MainWindow::createBoard()
{
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
        {
            auto button = new QPushButton;
            button->setFixedHeight(90);
            button->setFixedWidth(90);
            button->setFont(QFont("Times", 20, QFont::Bold));
            ui->gridLayout->addWidget(button, i, j);
            button->show();
            coords[button] = {i, j};
            connect(button, &QPushButton::clicked, this, &MainWindow::onCellClicked);
        }
}

void MainWindow::displayBoard(QVector<QVector<char> > &board)
{
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
        {
            auto button = dynamic_cast<QPushButton*>
                    (ui->gridLayout->itemAtPosition(i, j)->widget());
            button->setText((board[i][j] == '.') ? QString() : QChar(board[i][j]));
        }
}

void MainWindow::send(int i, int j)
{
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_3);
    out << QString("place") << i << j;
    socket->write(data);
}

void MainWindow::sendRematch()
{
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_3);
    out << QString("rematch");
    socket->write(data);
}


void MainWindow::on_connectButton_clicked()
{
    qDebug() << socket->state();
    if(socket->state() == QTcpSocket::ConnectedState)
    {
        socket->disconnectFromHost();
        socket->deleteLater();
        socket = new QTcpSocket;
    }
    else
    {
        QString ip = ui->ipLine->text();
        QString port = ui->portLine->text();
        ui->connectButton->setText("Connecting...");
        socket->connectToHost(ip, port.toInt());
    }

    if(socket->state() == QTcpSocket::ConnectingState || socket->state() == QTcpSocket::ConnectedState)
        ui->connectButton->setText("Disconnect");
    else
        ui->connectButton->setText("Connect");
}

void MainWindow::onCellClicked()
{
    auto pos = coords[dynamic_cast<QPushButton*>(sender())];
    send(pos.first, pos.second);
}

void MainWindow::slotReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_3);
    if(in.status() == QDataStream::Ok)
    {
        QVector<QVector<char>> board;
        int turn, win;
        in >> board >> turn >> win;
        displayBoard(board);
        if(win == 2)
            ui->turnLabel->setText("Draw");
        else if(win == 1)
            ui->turnLabel->setText(turn ? "You lost" : "You won");
        else
            ui->turnLabel->setText(turn ? "Your turn" : "Enemy turn");
        if(win)
        {
            ui->rematchButton->show();
            ui->rematchButton->setText("Rematch");
            ui->rematchButton->setEnabled(1);
        }
        else
            ui->rematchButton->hide();

    }
    else
    {
        qDebug() << "Error reading";
    }
}


void MainWindow::on_rematchButton_clicked()
{
    ui->rematchButton->setText("Offer sent");
    ui->rematchButton->setEnabled(0);
    sendRematch();
}

