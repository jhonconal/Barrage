﻿#include "xyfullscreenshots.h"
#include "xymenu.h"
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QFileDialog>
#include <QLineEdit>

XYFullScreenShots *XYFullScreenShots::mopInstance = NULL;
XYFullScreenShots::XYFullScreenShots(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowType_Mask | Qt::WindowStaysOnTopHint);
    setWindowState(windowState() | Qt::WindowFullScreen);
    mbLeftPress = false;
    mbMoveRect = false;
    moBackPixmap = qApp->screens().at(qApp->desktop()->screenNumber())->grabWindow(qApp->desktop()->winId());
    update();
}

XYFullScreenShots *XYFullScreenShots::getInstance()
{
    if (mopInstance == NULL)
    {
        mopInstance = new XYFullScreenShots;
    }
    return mopInstance;
}

XYFullScreenShots::~XYFullScreenShots()
{

}

void XYFullScreenShots::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, moBackPixmap);
    painter.setBrush(QColor("#99111100"));
    painter.drawRect(rect());
    painter.drawPixmap(moDrawPoint, moChoiseRectPixmap);
    painter.setPen(QColor("blue"));
    painter.setBrush(QColor("#00000000"));
    painter.drawRect(QRect(moDrawPoint, moChoiseRectPixmap.size()));

    if (!moChoiseRectPixmap.size().isEmpty())
    {
        painter.setPen(QColor("red"));
        int h = QFontMetrics(painter.font()).height();
        if (moDrawPoint.y() < 5 + h)
        {
            painter.drawText(moDrawPoint + QPoint(5, h), QString("( %1, %2 )")
                             .arg(moChoiseRectPixmap.width())
                             .arg(moChoiseRectPixmap.height()));
        }
        else
        {
            painter.drawText(moDrawPoint + QPoint(5, 0), QString("( %1, %2 )")
                             .arg(moChoiseRectPixmap.width())
                             .arg(moChoiseRectPixmap.height()));
        }
    }
}

void XYFullScreenShots::mousePressEvent(QMouseEvent *event)
{
    setFocus();

    if (event->button() == Qt::LeftButton)
    {
        mbLeftPress = true;
        moPressPoint = event->pos();
        if (moCurrentRect.contains(event->pos()))
        {
            mbMoveRect = true;
        }
    }
    else if (event->button() == Qt::RightButton && moCurrentRect.contains(event->pos()))
    {
        ShowMenu();
    }
}

void XYFullScreenShots::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton && !moCurrentRect.contains(event->pos()))
    {
        if (moChoiseRectPixmap.isNull())
        {
            close();
        }
        else
        {
            moChoiseRectPixmap.load("");
            moCurrentRect = QRect();
            update();
        }
    }
    else if (event->button() == Qt::LeftButton)
    {
        mbLeftPress = false;
        if (mbMoveRect)
        {
            mbMoveRect  = false;
        }
    }
}

void XYFullScreenShots::mouseMoveEvent(QMouseEvent *event)
{
    if (mbLeftPress && !mbMoveRect)
    {
        QPoint dire = event->pos() - moPressPoint;
        moDrawPoint = moPressPoint;
        if (dire.x() == 0 || dire.y() == 0)
        {
            moCurrentRect = QRect(moPressPoint, event->pos());
            moChoiseRectPixmap.load("");
        }
        else if (dire.x() > 0 && dire.y() > 0)
        {
            moCurrentRect = QRect(moPressPoint, event->pos());
            moChoiseRectPixmap = moBackPixmap.copy(QRect(moPressPoint, event->pos()));
        }
        else if (dire.x() > 0 && dire.y() < 0)
        {
            moCurrentRect = QRect(moPressPoint.x(),
                                  event->pos().y(),
                                  dire.x(),
                                  -dire.y());
            moChoiseRectPixmap = moBackPixmap.copy(QRect(moPressPoint.x(),
                                                         event->pos().y(),
                                                         dire.x(),
                                                         -dire.y()));
            moDrawPoint = QPoint(moPressPoint.x(), event->pos().y());
        }
        else if (dire.x() < 0 && dire.y() > 0)
        {
            moCurrentRect = QRect(event->pos().x(),
                                  moPressPoint.y(),
                                  -dire.x(),
                                  dire.y());
            moChoiseRectPixmap = moBackPixmap.copy(QRect(event->pos().x(),
                                                         moPressPoint.y(),
                                                         -dire.x(),
                                                         dire.y()));
            moDrawPoint = QPoint(event->pos().x(), moPressPoint.y());
        }
        else
        {
            moCurrentRect = QRect(event->pos(), moPressPoint);
            moChoiseRectPixmap = moBackPixmap.copy(QRect(event->pos(), moPressPoint));
            moDrawPoint = event->pos();
        }
        update();
    }
    else if (mbMoveRect && mbLeftPress)
    {
        moDrawPoint += event->pos() - moPressPoint;
        moPressPoint = event->pos();
        if (moDrawPoint.x() < 0)
        {
            moDrawPoint.setX(0);
        }
        else if (moDrawPoint.x() > width() - moChoiseRectPixmap.width())
        {
            moDrawPoint.setX(width() - moChoiseRectPixmap.width());
        }
        if (moDrawPoint.y() < 0)
        {
            moDrawPoint.setY(0);
        }
        else if (moDrawPoint.y() > height() - moChoiseRectPixmap.height())
        {
            moDrawPoint.setY(height() - moChoiseRectPixmap.height());
        }
        moCurrentRect = QRect(moDrawPoint, moCurrentRect.size());
        moChoiseRectPixmap = moBackPixmap.copy(QRect(moDrawPoint, moChoiseRectPixmap.size()));
        update();
    }
}

void XYFullScreenShots::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Up:
        moDrawPoint.setY(moDrawPoint.y() - 1);
        break;
    case Qt::Key_Down:
        moDrawPoint.setY(moDrawPoint.y() + 1);
        break;
    case Qt::Key_Left:
        moDrawPoint.setX(moDrawPoint.x() - 1);
        break;
    case Qt::Key_Right:
        moDrawPoint.setX(moDrawPoint.x() + 1);
        break;
    default:
        return;
    }
    moCurrentRect = QRect(moDrawPoint, moCurrentRect.size());
    moChoiseRectPixmap = moBackPixmap.copy(QRect(moDrawPoint, moChoiseRectPixmap.size()));
    update();
}

bool XYFullScreenShots::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    if(eventType == "windows_generic_MSG")
    {
        MSG *msg=static_cast<MSG*>(message);
        if(msg->message == WM_HOTKEY)
        {
            UINT fuModifiers = (UINT) LOWORD(msg->lParam);  // key-modifier flags
            UINT uVirtKey = (UINT) HIWORD(msg->lParam);     // virtual-key code

            // This is HotKey!;
            if(fuModifiers == MOD_SHIFT|MOD_ALT && uVirtKey == 0x41)
            {
                moBackPixmap = qApp->screens().at(qApp->desktop()->screenNumber())->grabWindow(qApp->desktop()->winId());
                show();
            }
            return true;

        }
    }
    return false;
}

void XYFullScreenShots::ShowMenu()
{
    XYMenu menu;
    QAction *save = new QAction(tr("Save"), &menu);
    connect(save, SIGNAL(triggered()), this, SLOT(saveChoisePixmap()));

    QAction *setRect = new QAction(tr("Set Rect"), &menu);
    connect(setRect, SIGNAL(triggered()), this, SLOT(resetChoiseRectSize()));

    menu.addAction(setRect);
    menu.addAction(save);

    menu.exec();
}

void XYFullScreenShots::saveChoisePixmap()
{
    QString imageFile = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                     QString(),
                                                     tr("PNG (*.png);;JPG (*.jpg);;BMP (*.bmp)"));


    if (imageFile.endsWith(".png"))
    {
        moChoiseRectPixmap.save(imageFile, "PNG");
    }
    else if (imageFile.endsWith(".jpg"))
    {
        moChoiseRectPixmap.save(imageFile, "JPG");
    }
    else if (imageFile.endsWith(".bmp"))
    {
        moChoiseRectPixmap.save(imageFile, "BMP");
    }
    else if (!imageFile.isEmpty())
    {
        moChoiseRectPixmap.save(imageFile);
    }

    moChoiseRectPixmap.load("");
    close();
}

void XYFullScreenShots::resetChoiseRectSize()
{
    int h = QFontMetrics(qApp->font()).height();
    QEventLoop loop;
    QLineEdit line(this);
    line.setFocus();
    line.setText(QString("(   %1   ,   %2   )").arg(moChoiseRectPixmap.width()).arg(moChoiseRectPixmap.height()));
    connect(&line, SIGNAL(editingFinished()), &loop, SLOT(quit()));
    line.move(moDrawPoint.x(), moDrawPoint.y() - h - 7);
    line.show();
    loop.exec();

    QString width;
    QString height;
    bool bw = true;
    bool st = false;
    for (int i = 0; i < line.text().size(); ++i)
    {
        if (line.text().at(i).isNumber())
        {
            st = true;
            if (bw)
            {
                width.append(line.text().at(i));
            }
            else
            {
                height.append(line.text().at(i));
            }
        }
        else if (st)
        {
            bw = false;
        }
    }
    moChoiseRectPixmap = moBackPixmap.copy(QRect(moDrawPoint, QSize(width.toInt(), height.toInt())));
    moCurrentRect = QRect(moDrawPoint, moChoiseRectPixmap.size());
    update();
}

