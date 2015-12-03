#ifndef TASKTABLEWIDGET_H
#define TASKTABLEWIDGET_H

#include <QTableWidget>

class TaskItemWidget;
class TaskItemData;
class DownloadTask;
class TaskTableWidget : public QTableWidget
{
	Q_OBJECT

public:
	TaskTableWidget(QWidget *parent);
	~TaskTableWidget();

	void createCellWidget(int row, const TaskItemData &data);
	TaskItemWidget* findItemWidget(const DownloadTask *task);

private:
	
};

#endif // TASKTABLEWIDGET_H
