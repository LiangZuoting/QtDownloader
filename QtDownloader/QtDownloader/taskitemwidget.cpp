#include "taskitemwidget.h"
#include "downloadtask.h"

TaskItemData::TaskItemData()
	: task(nullptr)
	, bytesPerSecond(0)
{}

TaskItemWidget::TaskItemWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

TaskItemWidget::~TaskItemWidget()
{

}

void TaskItemWidget::updateData(const TaskItemData &data)
{
	data_ = data;
	ui.name->setText(data_.task->name());
	ui.size->setText(bytesToDisplayString(data_.task->size()));
	ui.progressBar->setValue((float)data_.task->progress() / data_.task->size() * 100);
}

const TaskItemData& TaskItemWidget::itemData() const
{
	return data_;
}

QString TaskItemWidget::bytesToDisplayString(qint64 bytes)
{
	static QStringList units;
	units << "B" << "K" << "M" << "G" << "T";
	int index = 0;
	float fVal = bytes;
	for (; bytes >= 1024; ++index)
	{
		bytes /= 1024;
		fVal /= 1024;
	}
	index = qMin(index, units.size()-1);
	return QString::number(fVal, 'f', 1) + units[index];
}