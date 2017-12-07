#include <QMessageBox>
#include <QTimer>
#include <QMouseEvent>
#include <QDebug>
#include <QRectF>
#include <QString>
#include <QPainter>
#include "QTime"
#include <qmath.h>
#include "gamewidget.h"


GameWidget::GameWidget(QWidget *parent) :
    QWidget(parent),
    timer(new QTimer(this)),
    timerColor(new QTimer(this)),
    generations(-1),
    ca1(),
    universeSize(50),
    universeMode(0),
    cellMode(0)
    //randomMode(0)
    //lifeTime(50)
{
    timer->setInterval(300);
    timerColor->setInterval(50);
    masterColor = "#000";
    ca1.resetWorldSize(universeSize, universeSize);
    connect(timer, SIGNAL(timeout()), this, SLOT(newGeneration()));
    connect(timerColor, SIGNAL(timeout()), this, SLOT(newGenerationColor()));
}


GameWidget::~GameWidget() {
}


void GameWidget::startGame(const int &number) {
    /* start the game */
    generations = number;
    timer->start();
}


void GameWidget::stopGame() {
    /* stop the game */
    timer->stop();
    timerColor->stop();
}


void GameWidget::clearGame() {
    for (int k = 1; k <= universeSize; k++) {
        for (int j = 1; j <= universeSize; j++) {
            ca1.setValue(j, k, 0);
        }
    }
    gameEnds(true);
    ca1.resetWorldSize(universeSize, universeSize);
    //randomMode = 0;
    update();
}


int GameWidget::getUniverseSize() {
    return universeSize;
}


void GameWidget::setUniverseSize(const int &s) {
    /* set number of the cells in one row */
    universeSize = s;
    ca1.resetWorldSize(s, s);
    update();
}


int GameWidget::getUniverseMode() {
    return universeMode;
}


void GameWidget::setUniverseMode(const int &m) {
    /* set universe mode */
    int old_m = GameWidget::getUniverseMode();
    universeMode = m;

    if (old_m != m) {
        GameWidget::clearGame();
    }
    if (m == 1) {
        ca1.placeInitSnake();
        ca1.placeNewFood();
    update();
    }
}


int GameWidget::getCellMode() {
    return cellMode;
}


void GameWidget::setCellMode(const int &m) {
    /* set cell mode */
    cellMode = m;
}


CAbase GameWidget::getCA() {
    return ca1;
}


QString GameWidget::dumpGame() {
    /* dump current universe */
    char temp;
    QString master = "";

    switch (universeMode) {
    //
    // game of life
    //
    case 0:
        for (int k = 1; k <= universeSize; k++) {
            for (int j = 1; j <= universeSize; j++) {
                if (ca1.getValue(j, k) == 1) {
                    temp = '*';
                } else {
                    temp = 'o';
                }
                master.append(temp);
            }
            master.append("\n");
        }
        return master;
        break;

    //
    // snake
    //
    case 1:
        for (int k = 1; k <= universeSize; k++) {
            for (int j = 1; j <= universeSize; j++) {
                int value = ca1.getValue(j, k);
                if (value == 5) {
                    temp = 'F';
                } else if (value == 10) {
                    temp = 'H';
                } else if (value == 11) {
                    temp = 'B';
                } else if (value == ca1.snakeLength) {
                    temp = 'T';
                } else {
                    temp = 'o';
                }
                master.append(temp);
            }
            master.append("\n");
        }
        return master;
        break;

    default:
        return "";
        break;
    }

}


void GameWidget::reconstructGame(const QString &data) {
     // reconstruct game from dump
    int current;
    switch (universeMode) {
    //
    // game of life
    //
    case 0:
        current = 0;
        for (int k = 1; k <= universeSize; k++) {
            for (int j = 1; j <= universeSize; j++) {
               if (data[current] == '*') {
                   ca1.setValue(j, k, 1);
                }
                current++;
            }
            current++;
        }
        break;

    //
    // snake
    //
    case 1:
        current = 0;
        for (int k = 1; k <= universeSize; k++) {
            for (int j = 1; j <= universeSize; j++) {
                if (data[current] == 'F') {
                    ca1.setValue(j, k, 5);
                } else if (data[current] == 'H') {
                    ca1.setValue(j, k, 10);
                } else if (data[current] == 'B') {
                    ca1.setValue(j, k, 11);
                } else if (data[current] == 'T') {
                    ca1.setValue(j, k, ca1.snakeLength);
                } else {
                    ca1.setValue(j, k, 0);
                }
                current++;
            }
            current++;
        }
        break;

    default:
        break;
    }

    update();
}


int GameWidget::getInterval() {
    /* interval between generations */
    return timer->interval();
}


void GameWidget::setInterval(int msec) {
    /* set interval between generations */
    timer->setInterval(msec);
}


void GameWidget::newGeneration() {
    /* start the evolution of universe and update the game field */
    if (generations < 0)
        generations++;

    switch (universeMode) {
    //
    // game of life
    //
    case 0:
        ca1.worldEvolutionLife();
        break;
    //
    // snake
    //
    case 1:
        ca1.worldEvolutionLife();
        //ca1.worldEvolutionSnake();
        break;
    default:
        break;
    }

    update();

    if (ca1.isNotChanged()) {
        const QString headlines[] = {"Evolution stopped!", "Game over!"};
        const QString details[] = {"All future generations will be identical to this one.",
                             "Your snake made an illegal move."};

        QMessageBox msgBox;
        switch (universeMode) {
        case 0:
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(headlines[0]);
            msgBox.setInformativeText(details[0]);
        break;

        case 1:
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(headlines[1]);
            msgBox.setInformativeText(details[1]);

        default:
            break;
        }
        msgBox.exec();
        stopGame();
        gameEnds(true);
        return;
    }

    generations--;
    if (generations == 0) {
        stopGame();
        gameEnds(true);
        QMessageBox::information(this,
                                 tr("Game finished."),
                                 tr("Iterations finished."),
                                 QMessageBox::Ok,
                                 QMessageBox::Cancel);
    }

}


void GameWidget::newGenerationColor() {
    /* Start the evolution of universe and update the game field for "Cyclic cellular automata" mode */
    if (generations < 0)
        generations++;

    update();

    generations--;
    if (generations == 0) {
        stopGame();
        gameEnds(true);
        QMessageBox::information(this,
                                 tr("Game finished."),
                                 tr("Iterations finished."),
                                 QMessageBox::Ok,
                                 QMessageBox::Cancel);
    }
}


void GameWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    paintGrid(p);
    paintUniverse(p);
}


void GameWidget::mousePressEvent(QMouseEvent *e) {
    emit environmentChanged(true);
    double cellWidth = (double) width() / universeSize;
    double cellHeight = (double) height() / universeSize;
    int k = floor(e->y()/cellHeight) + 1;
    int j = floor(e->x()/cellWidth) + 1;

    int mode[9] = {1, 3, 6, 4, 2, 8, 9, 10, 11};

    if (universeMode == 0) {
        if (ca1.getValue(j, k) != 0) {
            ca1.setValue(j, k, 0);
            ca1.setLifetime(j, k, 0);
        }
        else {
            ca1.setValue(j, k, mode[cellMode]);
            if (mode[cellMode] == 9 || mode[cellMode] == 10)
                ca1.setLifetime(j, k, 50); // lifeTime = 50
        }
        update();
    }
}


void GameWidget::mouseMoveEvent(QMouseEvent *e) {
    double cellWidth = (double) width() / universeSize;
    double cellHeight = (double) height() / universeSize;
    int k = floor(e->y() / cellHeight) + 1;
    int j = floor(e->x() / cellWidth) + 1;

    int mode[9] = {1, 3, 6, 4, 2, 8, 9, 10, 11};

    if (universeMode == 0) {
        if (ca1.getValue(j, k) == 0) {
            ca1.setValue(j, k, mode[cellMode]);
            if (mode[cellMode] == 9 || mode[cellMode] == 10)
                ca1.setLifetime(j, k, 50); // lifetime = 50
            update();
        }
    }
}


int GameWidget::getSnakeDirection() {
    return ca1.snakeDirection;
}


void GameWidget::setSnakeDirection(int sD) {
    ca1.snakeDirection = sD;
}


void GameWidget::keyPressEvent(QKeyEvent *e) {

    int keyValue = e->key();
    if (keyValue == Qt::Key_Up) {
        GameWidget::setSnakeDirection(8);
    } else if (keyValue == Qt::Key_Down) {
        GameWidget::setSnakeDirection(2);
    } else if (keyValue == Qt::Key_Left) {
        GameWidget::setSnakeDirection(4);
    } else if (keyValue == Qt::Key_Right) {
        GameWidget::setSnakeDirection(6);
    } else {

    }
}


void GameWidget::paintGrid(QPainter &p) {
    QRect borders(0, 0, width() - 1, height() - 1); // borders of the universe
    QColor gridColor = masterColor; // color of the grid
    gridColor.setAlpha(10); // must be lighter than main color
    p.setPen(gridColor);
    double cellWidth = (double) width() / universeSize; // width of the widget / number of cells at one row
    for (double k = cellWidth; k <= width(); k += cellWidth)
        p.drawLine(k, 0, k, height());
    double cellHeight = (double) height() / universeSize; // height of the widget / number of cells at one row
    for (double k = cellHeight; k <= height(); k += cellHeight)
        p.drawLine(0, k, width(), k);
    p.drawRect(borders);
}


void GameWidget::paintUniverse(QPainter &p) {
    double cellWidth = (double) width() / universeSize;
    double cellHeight = (double) height() / universeSize;
    for (int k = 1; k <= universeSize; k++) {
        for (int j = 1; j <= universeSize; j++) {
            if (ca1.getValue(j, k) != 0) {
                qreal left = (qreal) (cellWidth * j - cellWidth); // margin from left
                qreal top  = (qreal) (cellHeight * k - cellHeight); // margin from top
                QRectF r(left, top, (qreal) cellWidth, (qreal) cellHeight);
                p.fillRect(r, QBrush(masterColor));
//                if (0 && universeMode != 7) { // randomMode = 0
//                    p.fillRect(r, setColor(ca1.getColor(j, k))); //fill cell with brush from random mode
//                 }
//                else {
//                    if (ca1.getValue(j, k) == 1 || universeMode == 7) {
//                        p.fillRect(r, QBrush(masterColor)); // fill cell with brush of main color
//                    }
//                    else {
//                        p.fillRect(r, setColor(ca1.getValue(j, k))); //fill cell with brush of cell type
//                    }
//                }
            }
        }
    }
}


QColor GameWidget::getMasterColor() {
    return masterColor;
}


void GameWidget::setMasterColor(const QColor &color) {
    masterColor = color;
    update();
}


QColor GameWidget::setColor(const int &color) {
    QColor cellColor[12]= {Qt::red,
                           Qt::darkRed,
                           Qt::green,
                           Qt::darkGreen,
                           Qt::blue,
                           Qt::darkBlue,
                           Qt::cyan,
                           Qt::darkCyan,
                           Qt::magenta,
                           Qt::darkMagenta,
                           Qt::yellow,
                           Qt::darkYellow};

    return cellColor[color];
}


//int GameWidget::getLifeInterval() {
//    /* cell's lifetime - number of step when cell is on the universe */
//    return lifeTime;
//}


//void GameWidget::setLifeInterval(const int &l) {
//    /* set lifetime for cell */
//    lifeTime = l;
//}

