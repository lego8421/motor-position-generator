#include "mainwindow.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent)
{
    QLineEdit **lineEdit = new QLineEdit*[5];

    for(int i=0; i<5; i++) {
        lineEdit[i] = new QLineEdit();
        lineEdit[i]->setValidator(new QDoubleValidator(this));
        lineEdit[i]->setAlignment(Qt::AlignRight);
        lineEdit[i]->setMinimumWidth(50);
    }

    _startPosition = lineEdit[0];
    _startVelocity = lineEdit[1];
    _endPosition = lineEdit[2];
    _endVelocity = lineEdit[3];
    _time = lineEdit[4];

    _startPosition->setText("0.0");
    _startVelocity->setText("0.0");
    _endPosition->setText("10.0");
    _endVelocity->setText("0.0");
    _time->setText("3.0");

    _button = new QPushButton(tr("generate(&G)"));
    connect(_button, SIGNAL(clicked(bool)), this, SLOT(buttonClicked()));

    _positionFormularLabel = new QLabel("position(t) = ");
    _velocityFormularLabel = new QLabel("velocity(t) = ");



    // layout
    QVBoxLayout *mainLayout = new QVBoxLayout;

    // start
    QHBoxLayout *hBoxLayout1 = new QHBoxLayout();
    hBoxLayout1->addWidget(new QLabel(tr("start position")));
    hBoxLayout1->addWidget(_startPosition);
    hBoxLayout1->addWidget(new QLabel(tr("(m),    ")));
    hBoxLayout1->addWidget(new QLabel(tr("start velocity")));
    hBoxLayout1->addWidget(_startVelocity);
    hBoxLayout1->addWidget(new QLabel(tr("(m/s)")));

    // end
    QHBoxLayout *hBoxLayout2 = new QHBoxLayout();
    hBoxLayout2->addWidget(new QLabel(tr("end position")));
    hBoxLayout2->addWidget(_endPosition);
    hBoxLayout2->addWidget(new QLabel(tr("(m),    ")));
    hBoxLayout2->addWidget(new QLabel(tr("end velocity")));
    hBoxLayout2->addWidget(_endVelocity);
    hBoxLayout2->addWidget(new QLabel(tr("(m/s)")));

    // time
    QHBoxLayout *hBoxLayout3 = new QHBoxLayout();
    hBoxLayout3->addWidget(new QLabel(tr("time")));
    hBoxLayout3->addWidget(_time);
    hBoxLayout3->addWidget(new QLabel(tr("(s)")));


    // Create chart view with the chart
    _chart = new QChart();
    _chartView = new QChartView(_chart, this);

    // Create layout for grid and detached legend
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(_chartView, 0, 1, 3, 1);


    // Set the title and show legend
    _chart->legend()->setVisible(true);
    _chart->legend()->setAlignment(Qt::AlignBottom);
    _chartView->setRenderHint(QPainter::Antialiasing);


    mainLayout->addLayout(hBoxLayout1);
    mainLayout->addLayout(hBoxLayout2);
    mainLayout->addLayout(hBoxLayout3);
    mainLayout->addWidget(_button);
    mainLayout->addWidget(_positionFormularLabel);
    mainLayout->addWidget(_velocityFormularLabel);
    mainLayout->addLayout(gridLayout,1);

    setLayout(mainLayout);

    buttonClicked();
}

void MainWidget::connectMarkers()
{
    // Connect all markers to handler
    const auto markers = _chart->legend()->markers();
    for (QLegendMarker *marker : markers) {
        // Disconnect possible existing connection to avoid multiple connections
        QObject::disconnect(marker, &QLegendMarker::clicked, this, &MainWidget::handleMarkerClicked);
        QObject::connect(marker, &QLegendMarker::clicked, this, &MainWidget::handleMarkerClicked);
    }
}

void MainWidget::disconnectMarkers()
{
    const auto markers = _chart->legend()->markers();
    for (QLegendMarker *marker : markers) {
        QObject::disconnect(marker, &QLegendMarker::clicked, this, &MainWidget::handleMarkerClicked);
    }
}

void MainWidget::handleMarkerClicked()
{
    QLegendMarker* marker = qobject_cast<QLegendMarker*> (sender());

    switch(marker->type()) {
        case QLegendMarker::LegendMarkerTypeXY: {

            // Toggle visibility of series
            marker->series()->setVisible(!marker->series()->isVisible());

            // Turn legend marker back to visible, since hiding series also hides the marker
            // and we don't want it to happen now.
            marker->setVisible(true);

            // Dim the marker, if series is not visible
            qreal alpha = 1.0;

            if (!marker->series()->isVisible())
                alpha = 0.5;

            QColor color;
            QBrush brush = marker->labelBrush();
            color = brush.color();
            color.setAlphaF(alpha);
            brush.setColor(color);
            marker->setLabelBrush(brush);

            brush = marker->brush();
            color = brush.color();
            color.setAlphaF(alpha);
            brush.setColor(color);
            marker->setBrush(brush);

            QPen pen = marker->pen();
            color = pen.color();
            color.setAlphaF(alpha);
            pen.setColor(color);
            marker->setPen(pen);

            break;
        }
    }
}

void MainWidget::buttonClicked()
{
    bool ok = false;
    double ps = _startPosition->text().toDouble(&ok);
    double vs = _startVelocity->text().toDouble(&ok);
    double pe = _endPosition->text().toDouble(&ok);
    double ve = _endVelocity->text().toDouble(&ok);
    double t = _time->text().toDouble(&ok);
    double dt = t / 1000.0;

    double ap[4] = {0.0, };
    double vp[4] = {0.0, };

    if(t < 0.0) {
        QMessageBox::warning(this, qApp->applicationDisplayName(), tr("time must be positive (t >= 0.0)"));
        _time->setFocus();
        return;
    }

    ap[0] = ps;
    ap[1] = vs;
    ap[2] = -(3.0/qPow(t,2))*ps - (2.0/t)*vs + (3.0/qPow(t,2))*pe - (1.0/t)*ve;
    ap[3] = (2.0/qPow(t,2))*ps + (1.0/qPow(t,2))*vs - (2.0/qPow(t,3))*pe + (1.0/qPow(t,2))*ve;

    vp[0] = ps;
    vp[1] = vs;
    vp[2] = -(3.0/qPow(t,2))*ps - (2.0/t)*vs + (3.0/qPow(t,2))*pe - (1.0/t)*ve;
    vp[3] = (2.0/qPow(t,2))*ps + (1.0/qPow(t,2))*vs - (2.0/qPow(t,3))*pe + (1.0/qPow(t,2))*ve;


    // remove series
    disconnectMarkers();

    for (QLineSeries *series : _series) {
        _chart->removeSeries(series);
    }
    _series.clear();


    // position
    QLineSeries *positionSeries = new QLineSeries();
    _series.append(positionSeries);
    positionSeries->setName(QString("position"));

    QList<QPointF> positionData;
    for (double i = 0.0; i < t; i+=dt) {
        positionData.append(QPointF(i, ap[3]*qPow(i,3) + ap[2]*qPow(i,2) + ap[1]*i + ap[0]));
    }

    positionSeries->append(positionData);
    _chart->addSeries(positionSeries);
    connect(positionSeries, SIGNAL(hovered(QPointF,bool)), this, SLOT(tooltip(QPointF,bool)));


    QString positionFormular = "position(t) = ";
    if(ap[3] != 0.0)
        positionFormular += QString::number(ap[3]) + "t^3 ";
    if(ap[2] != 0.0) {
        if(ap[2] > 0.0)
            positionFormular += "+";
        positionFormular += QString::number(ap[2]) + "t^2 ";
    }
    if(ap[1] != 0.0) {
        if(ap[1] > 0.0)
            positionFormular += "+";
        positionFormular += QString::number(ap[1]) + "t ";
    }
    if(ap[0] != 0.0) {
        if(ap[0] > 0.0)
            positionFormular += "+";
        positionFormular += QString::number(ap[0]);
    }

    _positionFormularLabel->setText(positionFormular);


    // velocuty
    QLineSeries *velocitySeries = new QLineSeries();
    _series.append(velocitySeries);
    velocitySeries->setName(QString("velocity"));

    QList<QPointF> velocityData;
    for (double i = 0.0; i < t; i+=dt) {
        velocityData.append(QPointF(i, 3*ap[3]*qPow(i,2) + 2*ap[2]*i + ap[1]));
    }

    velocitySeries->append(velocityData);
    _chart->addSeries(velocitySeries);
    connect(velocitySeries, SIGNAL(hovered(QPointF,bool)), this, SLOT(tooltip(QPointF,bool)));


    QString velocityFormular = "velocity(t) = ";
    if(3*ap[3] != 0.0)
        velocityFormular += QString::number(3*ap[3]) + "t^2 ";
    if(2*ap[2] != 0.0) {
        if(2*ap[2] > 0.0)
            velocityFormular += "+";
        velocityFormular += QString::number(2*ap[2]) + "t ";
    }
    if(ap[1] != 0.0) {
        if(ap[1] > 0.0)
            velocityFormular += "+";
        velocityFormular += QString::number(ap[1]);
    }

    _velocityFormularLabel->setText(velocityFormular);

    _chart->createDefaultAxes();
    _chart->setAcceptHoverEvents(true);

    connectMarkers();
}

void MainWidget::tooltip(QPointF point, bool state)
{
    if (_tooltip == 0)
        _tooltip = new Callout(_chart);

    if (state) {
        _tooltip->setText(QString("X: %1 \nY: %2 ").arg(point.x()).arg(point.y()));
        _tooltip->setAnchor(point);
        _tooltip->setZValue(11);
        _tooltip->updateGeometry();
        _tooltip->show();
    } else {
        _tooltip->hide();
    }
}
