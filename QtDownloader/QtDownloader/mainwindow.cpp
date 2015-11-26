#include "mainwindow.h"
#include <qt_windows.h>
#include <QMouseEvent>

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif

MainWindow::MainWindow(QWidget *parent)
	: QDialog(parent, 0)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
}

MainWindow::~MainWindow()
{

}

bool MainWindow::winEvent(MSG *message, long *result)
{
	if (message->message == WM_NCHITTEST)
	{
		QPoint cursorPos(GET_X_LPARAM(message->lParam), GET_Y_LPARAM(message->lParam));
		cursorPos = mapFromGlobal(cursorPos);
		*result = hitTest(cursorPos);
		return *result != HTCLIENT && *result != HTCAPTION;
	}
	else if (message->message == WM_LBUTTONDOWN)
	{
		QPoint cursorPos(GET_X_LPARAM(message->lParam), GET_Y_LPARAM(message->lParam));
		cursorPos = mapFromGlobal(cursorPos);
		if (hitTest(cursorPos) == HTCAPTION)
		{
			PostMessage(message->hwnd, WM_NCLBUTTONDOWN, HTCAPTION, message->lParam);
			return true;
		}
	}
	else if (message->message == WM_LBUTTONDBLCLK)
	{
		QPoint cursorPos(GET_X_LPARAM(message->lParam), GET_Y_LPARAM(message->lParam));
		cursorPos = mapFromGlobal(cursorPos);
		if (hitTest(cursorPos) == HTCAPTION)
		{
			showMaximized();
			return true;
		}
	}
	return false;
}
// 
// void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
// {
// 	QDialog::mouseDoubleClickEvent(event);
// }
// 
// void MainWindow::mouseMoveEvent(QMouseEvent *event)
// {
// 	if (event->buttons() && Qt::LeftButton)
// 	{
// 		PostMessage((HWND)winId(), WM_NCMOUSEMOVE, HTCAPTION, MAKELPARAM(event->globalX(), event->globalY()));
// 	}
// 	QDialog::mouseMoveEvent(event);
// }
// 
// void MainWindow::mousePressEvent(QMouseEvent *event)
// {
// 	QDialog::mousePressEvent(event);
// }
// 
// void MainWindow::mouseReleaseEvent(QMouseEvent *event)
// {
// 	QDialog::mouseReleaseEvent(event);
// }

void MainWindow::onClick()
{
	static int i = 0;
	ui.pushButton->setText(QString::number(i++));
}

long MainWindow::hitTest(const QPoint &pos)
{
	bool left = pos.x() <= 5;
	bool top = pos.y() <= 5;
	bool right = width() - pos.x() <= 5;
	bool bottom = height() - pos.y() <= 5;
	if (left && top)
	{
		return HTTOPLEFT;
	}
	else if (left && bottom)
	{
		return HTBOTTOMLEFT;
	}
	else if (right && top)
	{
		return HTTOPRIGHT;
	}
	else if (right && bottom)
	{
		return HTBOTTOMRIGHT;
	}
	else if (left)
	{
		return HTLEFT;
	}
	else if (top)
	{
		return HTTOP;
	}
	else if (right)
	{
		return HTRIGHT;
	}
	else if (bottom)
	{
		return HTBOTTOM;
	}
	else
	{
		auto child = childAt(pos);
		 if (!child || child != this)
		 {
			 return HTCAPTION;
		 }
	}
	return HTCLIENT;
}