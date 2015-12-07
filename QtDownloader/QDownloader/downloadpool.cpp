#include "downloadpool.h"

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