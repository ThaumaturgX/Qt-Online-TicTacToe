#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QHash>
#include <QPushButton>
#include <QPair>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void createBoard();
    void displayBoard(QVector<QVector<char>>&);
    void send(int i, int j);
    void sendRematch();

private slots:
    void on_connectButton_clicked();
    void onCellClicked();

    void on_rematchButton_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QByteArray data;
    QHash<QPushButton*, QPair<int, int>> coords;

public slots:
    void slotReadyRead();
};
#endif // MAINWINDOW_H
