#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QDialog>
#include "ui_mainwindow.h"

class MainWindow : public QDialog
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

protected:
	virtual bool winEvent( MSG *message, long *result);
// 	virtual void mouseDoubleClickEvent(QMouseEvent *event);
// 	virtual void mouseMoveEvent(QMouseEvent *event);
// 	virtual void mousePressEvent(QMouseEvent *event);
// 	virtual void mouseReleaseEvent(QMouseEvent *event);

private slots:
	void onClick();

private:
	long hitTest(const QPoint &pos);

	Ui::MainWindowClass ui;
};

#endif // MAINWINDOW_H
