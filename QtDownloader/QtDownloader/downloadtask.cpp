#include "downloadtask.h"
#include <QNetworkRequest>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

const QString DownloadTask::INFO_FILE_EXT("inf");
const QString DownloadTask::TASK_FILE_EXT("tsk");

DownloadTask::SpeedTest::SpeedTest()
	: downloadBytes_(0)
{}

void DownloadTask::SpeedTest::reset()
{
	currentMSecsSinceEpoch_ = 0;
	lastMSecsSinceEpoch_ = 0;
	downloadBytes_ = 0;
}

qint64 DownloadTask::SpeedTest::bytesPerSecond(qint64 currentMSecsSinceEpoch)
{
	currentMSecsSinceEpoch_ = currentMSecsSinceEpoch;
	qint64 result = 0;
	if (downloadBytes_ > 0)
	{
		result = (float)downloadBytes_ / (currentMSecsSinceEpoch_ - lastMSecsSinceEpoch_) * 1000;
	}
	lastMSecsSinceEpoch_ = currentMSecsSinceEpoch;
	downloadBytes_ = 0;
	return result;
}

DownloadTask::DownloadTask(QObject *parent, QNetworkAccessManager *netMgr)
	: QObject(parent)
	, size_(0)
	, progress_(0)
	, state_(Unvalid)
	, networkMgr_(netMgr)
{}

DownloadTask::~DownloadTask()
{}

void DownloadTask::start()
{
	if (url_.isEmpty())
	{
		qDebug() << __FUNCTION__ << " " << __LINE__ << "url is empty.";
		return;
	}
	state_ = Start;

	if (name_.isEmpty())
	{
		splitName();
	}
	
	QNetworkRequest request(url_);
	QString range("bytes=%1-");
	range = range.arg(progress_);
	request.setRawHeader("Range", range.toAscii());
	QNetworkReply *reply = networkMgr_->get(request);
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
	connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
	speedTest_.lastMSecsSinceEpoch_ = QDateTime::currentMSecsSinceEpoch();
}

void DownloadTask::start(const QString &url)
{
	url_ = url;
	start();
}

void DownloadTask::pause()
{
	state_ = Pause;
	speedTest_.reset();
}

void DownloadTask::cancel()
{
	state_ = Cancel;
}

void DownloadTask::setUrl(const QString &url)
{
	url_ = url;
}

void DownloadTask::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	qDebug() << QString(__FUNCTION__) << " bytesReceived:" << bytesReceived << " , bytesTotal:" << bytesTotal;
	QNetworkReply *reply = reinterpret_cast<QNetworkReply*>(sender());
	if (reply->error() != QNetworkReply::NoError)
	{
		qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << reply->error();
		return;
	}
	QFileInfo fi(infoFileFullName());
	qint64 bytesWritten = 0;
	if (!fi.exists())
	{
		size_ = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
		QFile taskFile(taskFileFullName());
		if (!taskFile.open(QFile::ReadWrite))
		{
			qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << taskFile.error();
			return;
		}
		if (!taskFile.resize(size_))
		{
			qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << taskFile.error();
			return;
		}
		QFile infoFile(infoFileFullName());
		if (!infoFile.open(QFile::ReadWrite))
		{
			qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << infoFile.error();
			return;
		}
		bytesWritten = infoFile.write((char*)&size_, sizeof(size_));
		if (bytesWritten != sizeof(size_))
		{
			qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << infoFile.error();
			return;
		}
	}
	QFile taskFile(taskFileFullName());
	if (!taskFile.open(QFile::ReadWrite))
	{
		qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << taskFile.error();
		return;
	}
	if (!taskFile.seek(progress_))
	{
		qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << taskFile.error();
		return;
	}
	qint64 size = reply->size();
	bytesWritten = taskFile.write(reply->readAll());
	if (bytesWritten != size)
	{
		qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << taskFile.error();
		return;
	}
	progress_ += size;

	QFile infoFile(infoFileFullName());
	if (!infoFile.open(QFile::ReadWrite))
	{
		qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << infoFile.error();
		return;
	}
	if (!infoFile.seek(sizeof(size_)))
	{
		qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << infoFile.error();
		return;
	}
	bytesWritten = infoFile.write((char*)&progress_, sizeof(progress_));
	if (bytesWritten != sizeof(progress_))
	{
		qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << infoFile.error();
		return;
	}
	speedTest_.downloadBytes_ += size;
	qint64 currentMsecs = QDateTime::currentMSecsSinceEpoch();
	qint64 timeSpan = currentMsecs - speedTest_.lastMSecsSinceEpoch_;
	qint64 bytesPerSecond = 0;
	if (timeSpan > 50)	//每50ms通知一次UI
	{
		bytesPerSecond = speedTest_.bytesPerSecond(currentMsecs);
		emit downloadProgress(progress_, size_, bytesPerSecond);
		qDebug() << "kbytes per second : " << bytesPerSecond / 1024.0f << " timespan : " << timeSpan;
	}
	if (state_ == Pause || state_ == Cancel)
	{
		reply->deleteLater();
		if (state_ == Cancel)
		{
			QFile::remove(infoFileFullName());
			QFile::remove(taskFileFullName());
		}
	}
}

void DownloadTask::onFinished()
{
	qDebug() << QString(__FUNCTION__);
	emit finished();
	QNetworkReply *reply = reinterpret_cast<QNetworkReply*>(sender());
	if (reply->error() == QNetworkReply::NoError)
	{
		state_ = Finish;
		reply->deleteLater();
		QString taskFileName = taskFileFullName();
		QFile taskFile(taskFileName);
		taskFileName = taskFileName.left(taskFileName.size() - TASK_FILE_EXT.size());
		if (!taskFile.rename(taskFileName))
		{
			qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << taskFile.error();
		}
		QFile infoFile(infoFileFullName());
		if (!infoFile.remove())
		{
			qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << infoFile.error();
		}
	}
}

void DownloadTask::onError(QNetworkReply::NetworkError code)
{
	emit error(code);
	state_ = Error;
	QNetworkReply *reply = reinterpret_cast<QNetworkReply*>(sender());
	qDebug() << "error : " << reply->errorString();
	QFile::remove(infoFileFullName());
	QFile::remove(taskFileFullName());
}

void DownloadTask::splitName()
{
	if (url_.isEmpty())
	{
		qDebug() << __FUNCTION__ << " " << __LINE__ << "url is empty.";
		return;
	}
	int index = url_.lastIndexOf('/');
	if (index == -1)
	{
		qDebug() << __FUNCTION__ << " " << __LINE__ << "url is wrong.";
		return;
	}
	name_ = url_.right(url_.size() - index -1);
}

QString DownloadTask::infoFileFullName()
{
	QString result = path_;
	if (!result.endsWith('\\'))
	{
		result += "\\";
	}
	result += name_ + "." + INFO_FILE_EXT;
	return result;
}

QString DownloadTask::taskFileFullName()
{
	QString result = path_;
	if (!result.endsWith('\\'))
	{
		result += "\\";
	}
	result += name_ + "." + TASK_FILE_EXT;
	return result;
}