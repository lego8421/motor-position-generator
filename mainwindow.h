#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QtCharts>

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    MainWidget(QWidget *parent = 0);

public slots:
    void connectMarkers();
    void disconnectMarkers();
    void handleMarkerClicked();
    void buttonClicked();

private:
    QLineEdit *_startPosition;
    QLineEdit *_endPosition;
    QLineEdit *_startVelocity;
    QLineEdit *_endVelocity;
    QLineEdit *_time;
    QPushButton *_button;

    QLabel *_positionFormularLabel;
    QLabel *_velocityFormularLabel;

    QChart *_chart;
    QList<QLineSeries *> _series;
    QChartView *_chartView;
};

#endif // MAINWINDOW_H
