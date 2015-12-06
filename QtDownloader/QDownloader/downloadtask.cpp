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


DownloadTask::DownloadTask(QObject *parent)
	: QObject(parent)
	, size_(0)
	, progress_(0)
	, state_(Unvalid)
	, networkMgr_(nullptr)
	, infoFile_(this)
	, taskFile_(this)
{}

DownloadTask::~DownloadTask()
{}

void DownloadTask::init(const QString &savePath, const QString &url, QNetworkAccessManager *netMgr)
{
	Q_ASSERT(state_ == Unvalid);
	state_ = Init;
	path_ = savePath;
	url_ = url;
	networkMgr_ = netMgr;
	splitName();
	infoFile_.setFileName(infoFileFullName());
	if (!infoFile_.open(QFile::ReadWrite))
	{
		qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << infoFile_.error();
		return;
	}
	if (infoFile_.size() != 0)
	{
		infoFile_.read((char*)&size_, sizeof(size_));
		infoFile_.read((char*)&progress_, sizeof(progress_));
	}
	emit inited();
}

void DownloadTask::start()
{
	Q_ASSERT(state_ != Unvalid);
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

QString DownloadTask::url() const
{
	return url_;
}

void DownloadTask::setName(const QString &name)
{
	name_ = name;
}

QString DownloadTask::name() const
{
	return name_;
}

void DownloadTask::setPath(const QString &path)
{
	path_ = path;
}

QString DownloadTask::path() const
{
	return path_;
}

void DownloadTask::setSize(qint64 size)
{
	size_ = size;
}

qint64 DownloadTask::size() const
{
	return size_;
}

void DownloadTask::setProgress(qint64 progress)
{
	progress_ = progress;
}

qint64 DownloadTask::progress() const
{
	return progress_;
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
	qint64 bytesWritten = 0;
	if (infoFile_.size() == 0)
	{
		size_ = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
		taskFile_.setFileName(taskFileFullName());
		//if inf file doesnt exist, task file has to be truncated.
		if (!taskFile_.open(QFile::ReadWrite | QFile::Truncate))
		{
			qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << taskFile_.error();
			return;
		}
		if (!taskFile_.resize(size_))
		{
			qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << taskFile_.error();
			return;
		}
		bytesWritten = infoFile_.write((char*)&size_, sizeof(size_));
		if (bytesWritten != sizeof(size_))
		{
			qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << infoFile_.error();
			return;
		}
	}
	if (!taskFile_.isOpen())
	{
		taskFile_.setFileName(taskFileFullName());
		if (!taskFile_.open(QFile::ReadWrite))
		{
			qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << taskFile_.error();
			return;
		}
	}
	if (!taskFile_.seek(progress_))
	{
		qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << taskFile_.error();
		return;
	}
	qint64 size = reply->size();
	bytesWritten = taskFile_.write(reply->readAll());
	if (bytesWritten != size)
	{
		qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << taskFile_.error();
		return;
	}
	progress_ += size;

	if (!infoFile_.seek(sizeof(size_)))
	{
		qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << infoFile_.error();
		return;
	}
	bytesWritten = infoFile_.write((char*)&progress_, sizeof(progress_));
	if (bytesWritten != sizeof(progress_))
	{
		qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << infoFile_.error();
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
			infoFile_.remove();
			taskFile_.remove();
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
		taskFileName = taskFileName.left(taskFileName.size() - TASK_FILE_EXT.size());
		if (!taskFile_.rename(taskFileName))
		{
			qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << taskFile_.error();
		}
		if (!infoFile_.remove())
		{
			qDebug() << QString(__FUNCTION__) << " " << __LINE__ << " error : " << infoFile_.error();
		}
	}
}

void DownloadTask::onError(QNetworkReply::NetworkError code)
{
	emit error(code);
	state_ = Error;
	QNetworkReply *reply = reinterpret_cast<QNetworkReply*>(sender());
	qDebug() << "error : " << reply->errorString();
	infoFile_.remove();
	taskFile_.remove();
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