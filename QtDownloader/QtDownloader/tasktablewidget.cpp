#include "tasktablewidget.h"
#include <QHeaderView>
#include "taskitemwidget.h"
#include "downloadtask.h"

TaskTableWidget::TaskTableWidget(QWidget *parent)
	: QTableWidget(parent)
{
	horizontalHeader()->setResizeMode(QHeaderView::Stretch);
}

TaskTableWidget::~TaskTableWidget()
{

}

void TaskTableWidget::createCellWidget(int row, const TaskItemData &data)
{
	TaskItemWidget *item = new TaskItemWidget(this);
	item->updateData(data);
	insertRow(row);
	setCellWidget(row, 0, item);
}

TaskItemWidget* TaskTableWidget::findItemWidget(const DownloadTask *task)
{
	for (int i = 0; i < rowCount(); ++i)
	{
		TaskItemWidget *item = static_cast<TaskItemWidget*>(cellWidget(i, 0));
		if (item)
		{
			auto data = item->itemData();
			if (data.task == task)
			{
				return item;
			}
		}
	}
	return nullptr;
}