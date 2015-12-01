#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QDialog>
#include "ui_mainwindow.h"
#include <QNetworkAccessManager>

class MainWindow : public QDialog
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

protected:
	virtual bool winEvent( MSG *message, long *result);
	virtual void showEvent(QShowEvent *e);
	virtual void keyPressEvent(QKeyEvent *e);

private slots:
	void onMinimize();
	void onMaximize();

private:
	long hitTest(const QPoint &pos);

	Ui::MainWindowClass ui;
	QNetworkAccessManager mgr_;
};

#endif // MAINWINDOW_H
