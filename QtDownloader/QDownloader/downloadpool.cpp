#include "downloadpool.h"
#include <QDir>
#include <QDateTime>
#include <QNetworkAccessManager>
#include "downloadtask.h"

DownloadPool::DownloadPool(QObject *parent)
	: QObject(parent)
	, maxDownloadCount_(5)
	, currentDownloadCount_(0)
{

}

DownloadPool::~DownloadPool()
{

}

void DownloadPool::init()
{}

DownloadPool& DownloadPool::instance()
{
	static DownloadPool p(nullptr);
	return p;
}

void DownloadPool::load(const QString &path, QNetworkAccessManager *netMgr)
{
	QDir dir(path);
	QStringList nameFilters;
	nameFilters << "*.inf";
	QFileInfoList fil = dir.entryInfoList(nameFilters, QDir::Files);
	qSort(fil.begin(), fil.end(), [](const QFileInfo &lhs, const QFileInfo &rhs){return lhs.created() < rhs.created();});
	for (int i = 0; i < fil.size(); ++i)
	{
		DownloadTask *task = new DownloadTask(this);
		connect(task, SIGNAL(finished()), this, SLOT(onFinished()));
		//Task should write last state to inf file so that pool restore task to that state.
		task->load(path, fil[i].fileName(), netMgr);
		lstTask_.push_back(task);

		++currentDownloadCount_;
		task->start();
	}
}

void DownloadPool::onFinished()
{
	lstTask_.removeOne(static_cast<DownloadTask*>(sender()));
	--currentDownloadCount_;
}