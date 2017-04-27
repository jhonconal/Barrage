﻿#include "csignalbarragescreen.h"
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QTime>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QDebug>
CSignalBarrageScreen::CSignalBarrageScreen(CBarrageItem *item, QWidget *parent)
    : CMouseMonitorWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint
                         | Qt::WindowStaysOnTopHint
                         | Qt::WindowType_Mask);
    this->setAttribute(Qt::WA_TranslucentBackground);

    mbLeftMousePressed = false;
    mbFixed = false;
    mbAutoMove = false;
    mopMoveAnimation = NULL;
    mopStepAnimation = NULL;
    mopContentsItem = NULL;
    miMoveAutoTimer = 0;
    setItem(item);
    miRefreshTimer = startTimer(30);
}

CSignalBarrageScreen::~CSignalBarrageScreen()
{
    delete mopContentsItem;
}

qreal CSignalBarrageScreen::step() const
{
    return mfStep;
}

void CSignalBarrageScreen::setStep(qreal step)
{
    if (mfStep != step)
    {
        mfStep = step;
        QPoint point = moMovePath.getPoint(step);
        if (point.x() > QApplication::desktop()->width() - 50)
        {
            point.setX(QApplication::desktop()->width() - 50);
        }
        else if (point.x() < 0)
        {
            point.setX(0);
        }

        if (point.y() > QApplication::desktop()->height() - 100)
        {
            point.setY(QApplication::desktop()->height() - 100);
        }
        else if (point.y() < 0)
        {
            point.setY(0);
        }
        move(point);
    }
}

void CSignalBarrageScreen::setItem(CBarrageItem *item)
{
    if (mopContentsItem != NULL)
    {
        delete mopContentsItem;
    }
    mopContentsItem = item;
    if (mopContentsItem)
    {
        int width = mopContentsItem->miBarrageWidth;
        if (width == -1)
        {
            width = 0;
            Contents *contents = mopContentsItem->mopContents;
            int w = 0;
            while (contents)
            {
                switch (contents->type)
                {
                case Contents::TEXT:
                    w += mopContentsItem->moTextFont.pointSize()
                             * contents->text.toLocal8Bit().length();
                    break;
                case Contents::LF:
                    width = qMax(w, width);
                    w = 0;
                    break;
                case Contents::PIXMAP:
                    if (!contents->pixmap.isValid())
                    {
                        QString filename = contents->pixmap.fileName();
                        w += QPixmap(filename).width();
                    }
                    else if (contents->pixmap.state() != QMovie::Running)
                    {
                        contents->pixmap.start();
                        w += contents->pixmap.currentPixmap().width();
                    }
                    else
                    {
                        w += contents->pixmap.currentPixmap().width();
                    }
                    break;
                default:
                    break;
                }
                contents = contents->next;
            }
        }
        int height = mopContentsItem->miBarrageHeight;
        if (height == -1)
        {
            height = 0;
            Contents *contents = mopContentsItem->mopContents;
            int h = 0;
            while (contents)
            {
                switch (contents->type)
                {
                case Contents::TEXT:
                    h = qMax(h, mopContentsItem->moTextFont.pointSize());
                    break;
                case Contents::LF:
                    height += h;
                    h = 0;
                    break;
                case Contents::PIXMAP:
                    if (!contents->pixmap.isValid())
                    {
                        QString filename = contents->pixmap.fileName();
                        h = qMax(h, QPixmap(filename).height());
                    }
                    else if (contents->pixmap.state() != QMovie::Running)
                    {
                        contents->pixmap.start();
                        h = qMax(h, contents->pixmap.currentPixmap().height());
                    }
                    else
                    {
                        h = qMax(h, contents->pixmap.currentPixmap().height());
                    }
                    break;
                default:
                    break;
                }
                contents = contents->next;
            }
        }
        resize(width, height);
        move(QApplication::desktop()->width() - width - 20
             , QApplication::desktop()->height() - height - 40);
    }
}

void CSignalBarrageScreen::changeFixed()
{
    mbFixed = !mbFixed;
    if (mbFixed)
    {
        if (mopMoveAnimation && mopMoveAnimation->state() == QPropertyAnimation::Running)
        {
            mopMoveAnimation->stop();
        }
    }
}

void CSignalBarrageScreen::changeAutoMove()
{
    mbAutoMove = !mbAutoMove;
    if (mbAutoMove)
    {
        if (miMoveAutoTimer == 0)
        {
            miMoveAutoTimer = startTimer(7000);
        }
    }
    else
    {
        killTimer(miMoveAutoTimer);
        miMoveAutoTimer = 0;
    }
}

void CSignalBarrageScreen::moveNextPoint()
{
    qsrand(QTime::currentTime().msecsTo(QTime(0, 0)) + (int)this);
    if (mopStepAnimation == NULL)
    {
        mopStepAnimation = new QPropertyAnimation(this, "step");
    }

    // 随机产生轨迹
    {
        int index = qrand() % 4;
        QPoint curpos = pos();
        switch (index)
        {
        case 0:
            moMovePath.setLine(curpos, QPoint(qrand() % QApplication::desktop()->width(),
                                             qrand() % QApplication::desktop()->height()));
            break;
        case 1:
            moMovePath.setCurve(curpos, QPoint(qrand() % QApplication::desktop()->width(),
                                             qrand() % QApplication::desktop()->height()));
            break;
        case 2:
            moMovePath.setTrigon(curpos, QPoint(qrand() % QApplication::desktop()->width(),
                                                qrand() % QApplication::desktop()->height())
                                 , QPoint(qrand() % QApplication::desktop()->width(),
                                          qrand() % QApplication::desktop()->height()));
            break;
        case 3:
            moMovePath.setRect(curpos, QPoint(curpos.x() + qrand() % 1000 - 500,
                                              curpos.y() + qrand() % 1000 - 500));
            break;
        case 4:
            moMovePath.setCentre(curpos, QPoint(curpos.x() + qrand() % 1000 - 500,
                                                curpos.y() + qrand() % 1000 - 500));
            break;
        }
    }

    mopStepAnimation->stop();
    mopStepAnimation->setStartValue(0);
    mopStepAnimation->setEndValue(1);
    mopStepAnimation->setDuration(qrand()%5 * 1000 + 5000);
    mopStepAnimation->setEasingCurve(QEasingCurve::Type(qrand() % 40));
    mopStepAnimation->start();
}

void CSignalBarrageScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter;
    painter.begin(this);
    CBarrageItem *item = mopContentsItem;
    painter.setPen(item->moTextColor);
    painter.setFont(item->moTextFont);
    if (item->miBarrageWidth == -1)
    {
        item->miBarrageWidth = QApplication::desktop()->width();
    }
    if (item->miBarrageHeight == -1)
    {
        item->miBarrageHeight = QApplication::desktop()->height();
    }

    QPixmap pixmap = item->moBackImage.currentPixmap();
    if (!pixmap.isNull())
    {
        painter.drawPixmap(QRect(0,
                                 0,
                                 qMin(item->miBarrageWidth, pixmap.width()),
                                 qMin(item->miBarrageHeight, pixmap.height())),
                           pixmap,
                           pixmap.rect());
    }
    Contents *contents = item->mopContents;
    qreal offset_X = 0;
    qreal offset_Y = 0;
    qreal current_Y = 0;
    int limit_X = 0;
    int limit_Y = 0;
    while (contents)
    {
        limit_X = item->miBarrageWidth - offset_X;
        limit_Y = item->miBarrageHeight - offset_Y;
        switch (contents->type)
        {
        case Contents::TEXT:
            painter.drawText(QRect(offset_X,
                                   offset_Y,
                                   limit_X < 0 ? 0:limit_X,
                                   limit_Y < 0 ? 0:limit_Y),
                             contents->text);
            offset_X += contents->text.toLatin1().length()
                    * item->moTextFont.pointSize() + 2;
            current_Y = qMax(current_Y, item->moTextFont.pointSize() + 2.0);
            break;
        case Contents::LF:
            offset_Y += current_Y;
            offset_X = 0;
            break;
        case Contents::PIXMAP:
        {
            QPixmap pixmap;
            if (!contents->pixmap.isValid())
            {
                QString filename = contents->pixmap.fileName();
                pixmap.load(filename);
            }
            else if (contents->pixmap.state() != QMovie::Running)
            {
                contents->pixmap.start();
                pixmap = contents->pixmap.currentPixmap();
            }
            else
            {
                pixmap = contents->pixmap.currentPixmap();
            }

            painter.drawPixmap(QRect(offset_X,
                                     offset_Y,
                                     qMin(limit_X < 0 ? 0:limit_X, pixmap.width()),
                                     qMin(limit_Y < 0 ? 0:limit_Y, pixmap.height())),
                               pixmap,
                               QRect(0,
                                     0,
                                     qMin(limit_X < 0 ? 0:limit_X, pixmap.width()),
                                     qMin(limit_Y < 0 ? 0:limit_Y, pixmap.height())));
            offset_X += pixmap.width() + 2;
            current_Y = qMax(current_Y, pixmap.height() + 2.0);
            break;
        }
        default:
            break;
        }

        contents = contents->next;
    }

    painter.end();
}

void CSignalBarrageScreen::timerEvent(QTimerEvent *event)
{
    if (miRefreshTimer == event->timerId())
    {
        update();
    }
    else if (miMoveAutoTimer == event->timerId())
    {
        if (!mbFixed && !mbLeftMousePressed)
        {
            moveNextPoint();
        }
    }
}

void CSignalBarrageScreen::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        setCurrentInstance(this);
        if (mopMoveAnimation && mopMoveAnimation->state() == QPropertyAnimation::Running)
        {
            mopMoveAnimation->stop();
        }
        if (mopStepAnimation && mopStepAnimation->state() == QPropertyAnimation::Running)
        {
            mopStepAnimation->stop();
        }
        mbLeftMousePressed = true;
        moLastPos = event->globalPos();
    }
}

void CSignalBarrageScreen::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        mbLeftMousePressed = false;
        moLastPos = event->globalPos();
    }
}

void CSignalBarrageScreen::mouseMoveEvent(QMouseEvent *event)
{
    if (mbLeftMousePressed)
    {
        QPoint lastpos = pos();
        lastpos.setX( lastpos.x() + event->globalX() - moLastPos.x());
        lastpos.setY( lastpos.y() + event->globalY() - moLastPos.y());
        move(lastpos);
        moLastPos = event->globalPos();
    }
}

void CSignalBarrageScreen::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QAction *exit = new QAction("Exit", &menu);
    connect(exit, SIGNAL(triggered()), this, SLOT(close()));
    connect(exit, SIGNAL(triggered()), this, SLOT(deleteLater()));

    QAction *fixed = new QAction("Fixed", &menu);
    fixed->setCheckable(true);
    fixed->setChecked(mbFixed);
    connect(fixed, SIGNAL(triggered()), this, SLOT(changeFixed()));

    QAction *automove = new QAction("AutoMove", &menu);
    automove->setCheckable(true);
    automove->setChecked(mbAutoMove);
    connect(automove, SIGNAL(triggered()), this, SLOT(changeAutoMove()));

    QAction *random = new QAction("Random", &menu);
    random->setCheckable(true);
    random->setChecked(mbRandomCall);
    connect(random, SIGNAL(triggered()), this, SLOT(changeRandomCall()));

    menu.addAction(random);
    menu.addAction(automove);
    menu.addAction(fixed);
    menu.addAction(exit);
    menu.move(pos().x() + event->x(), pos().y() + event->y());
    menu.exec();
}

void CSignalBarrageScreen::clicked(QPoint point)
{
    if (!mbLeftMousePressed && !mbFixed && !mbAutoMove)
    {
        qsrand(QTime::currentTime().msecsTo(QTime(0, 0)) + (int)this);
        if (mopMoveAnimation == NULL)
        {
            mopMoveAnimation = new QPropertyAnimation(this, "pos");
        }
        else
        {
            mopMoveAnimation->stop();
        }

        mopMoveAnimation->setEndValue(point);
        mopMoveAnimation->setDuration(5000);
        mopMoveAnimation->setEasingCurve(QEasingCurve::Type(qrand() % 40));
        mopMoveAnimation->start();
    }
}
