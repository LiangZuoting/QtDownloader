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
