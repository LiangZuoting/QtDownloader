#ifndef TASKITEMWIDGET_H
#define TASKITEMWIDGET_H

#include <QWidget>
#include "ui_taskitemwidget.h"

class DownloadTask;
class TaskItemData
{
public:
	TaskItemData();

	DownloadTask *task;
	qint64 bytesPerSecond;
};

class TaskItemWidget : public QWidget
{
	Q_OBJECT

public:
	TaskItemWidget(QWidget *parent = 0);
	~TaskItemWidget();

	void updateData(const TaskItemData &data);
	const TaskItemData& itemData() const;

private:
	QString bytesToDisplayString(qint64 bytes);

	Ui::TaskItemWidget ui;
	TaskItemData data_;
};

#endif // TASKITEMWIDGET_H
