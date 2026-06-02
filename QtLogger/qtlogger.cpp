#include "qtlogger.h"
#include <QRegularExpression>
#include <windows.h>

QtLogger* myPointer;

ThreadLog::ThreadLog()
{

}
ThreadLog::~ThreadLog()
{
	myPointer = nullptr;
}

void ThreadLog::writeLogThread(int logType, QString strLogInfo, int logPriority)
{
	if (myPointer)
		myPointer->WriteReallyLog(logType, strLogInfo, logPriority);
	return;
}

QtLogger::QtLogger(QObject* parent)
	: QObject(parent)
{
	logFileDir = QCoreApplication::applicationDirPath() + "/log/";
	myPointer = this;
	initSignalThread();
}

QtLogger::~QtLogger()
{
	myPointer = nullptr;
	m_reallThread->quit();
	m_reallThread->wait();
}

void QtLogger::WriteLog(QString strLogInfo, int logType, int logPriority)
{
	if (!myPointer)
	{
		QObject* app = QCoreApplication::instance();
		if (app)
			myPointer = new QtLogger(app);
		else
			myPointer = new QtLogger();
	}

	if (myPointer)
	{
		emit myPointer->writeLogSignal(logType, strLogInfo, logPriority);
	}
	return;
}

/**
作用：日志等级分类
 1.static Level EDEBUG :
    EDEBUG Level指出细粒度信息事件对调试应用程序是非常有帮助的。		=>0
 2.static Level INFO
    INFO level表明 消息在粗粒度级别上突出强调应用程序的运行过程。		=>1
 3.static Level WARN
    WARN level表明会出现潜在错误的情形。							=>2
 4.static Level Critical
    Critical level指出虽然发生错误事件，但仍然不影响系统的继续运行。	=>3
 5.static Level FATAL
    FATAL level指出每个严重的错误事件将会导致应用程序的退出。		=>4
*/
void QtLogger::WriteReallyLog(int logType, QString strLogInfo, int logPriority)
{

	QString  strType = "";
	if (logType == enLogType::EDEBUG)
	{
		strType = QObject::tr("[调试] ");
	}
	else if (logType == enLogType::WARNING)
	{
		strType = QObject::tr("[警告] ");
	}
	else if (logType == enLogType::SERIOUS)
	{
		strType = QObject::tr("[严重] ");
	}
	else if (logType == enLogType::DEADLY)
	{
		strType = QObject::tr("[致命] ");
	}
	else if (logType == enLogType::OPERATER)
	{
		strType = QObject::tr("[操作] ");
	}


	strLogInfo = strLogInfo.replace("\r", " ");
	strLogInfo = strLogInfo.replace("\n", " ");
	strLogInfo = strLogInfo.replace("\t\t\t", "\t");

	QMutexLocker locker(&mutex);
	QString logFileDir = QCoreApplication::applicationDirPath() + "/log/";
	QDateTime DT = QDateTime::currentDateTime();
	QString dataTime = QString("%1").arg(DT.toString("yyyy-MM-dd hh:mm:ss"));
	QString DTStr = QString("%1").arg(DT.toString("yyyy-MM-dd"));
	QString strLogContext = "";

	strLogContext = QObject::tr("%1 %2:%3").arg(dataTime).arg(strType).arg(strLogInfo);

	QString logFileName = DTStr + ".log";
	QDir dir;
	if (!dir.exists(logFileDir))
	{
		dir.mkpath(logFileDir);
	}
	QString logFilePath = logFileDir + logFileName;
	QFile logFile(logFilePath);
	if (logFile.open(QIODevice::ReadWrite | QIODevice::Append | QFile::Text))
	{
		// 如果文件是新建的（大小为0），先写入 UTF-8 BOM，这样 Windows 记事本能正确识别为 UTF-8
		if (logFile.size() == 0)
		{
			const QByteArray utf8Bom("\xEF\xBB\xBF");
			logFile.write(utf8Bom);
			logFile.flush();
		}

		QTextStream logStream(&logFile);
		// 明确使用 UTF-8 编码写入（Qt5 中可用）
		QTextCodec* codec = QTextCodec::codecForName("UTF-8");
		if (codec)
			logStream.setCodec(codec);
		else
			logStream.setCodec("UTF-8");

		logStream << (strLogContext) << endl;
		logFile.flush();
		logFile.close();
	}

	logPriority = 0;
	return;
}

void QtLogger::initSignalThread()
{
	m_signalThreadLog = new ThreadLog;
	m_reallThread = new QThread;
	connect(this, SIGNAL(writeLogSignal(int, QString, int)), m_signalThreadLog, SLOT(writeLogThread(int, QString, int)), Qt::DirectConnection);
	m_signalThreadLog->moveToThread(m_reallThread);
	m_reallThread->start();
}

QString QtLogger::GetDecimalLen(QString strSrc, int iLen)
{
	QString strData = strSrc;
	//取小数点后N位即可
	int m = strData.indexOf(".");
	if (strData.length() - m > iLen)
	{
		strData = strData.left(m + iLen + 1);
	}
	return strData;
}

bool QtLogger::FileCopy(QString sourceDir, QString toDir, QString sourName, QString toDirName, bool coverFileIfExist)
{
	sourceDir.replace("\\", "/");
	toDir.replace("\\", "/");

	QString secondC = toDir.mid(1, 1);
	if (secondC != "/" && secondC != ":")
	{
		QtLogger::WriteLog(QObject::tr("目标路径：%1，不符合拷贝路径的规范，第二个字符为：%2").arg(toDir).arg(secondC));
		return false;
	}

	QString  strSPath = "";
	if (sourceDir.right(1) != '/')
	{
		strSPath = sourceDir + "/" + sourName;
	}
	else
	{
		strSPath = sourceDir + sourName;
	}

	QString	strDecPath = "";
	if (strDecPath.right(1) != '/')
	{
		strDecPath = toDir + "/" + toDirName;
	}
	else
	{
		strDecPath = toDir + toDirName;
	}

	if (!isFileExist(strSPath))
	{
		QString  strError = QObject::tr("指定路径%1不存在").arg(strSPath);
		WriteLog(strError);
		return false;
	}
	toDir.replace("\\", "/");
	if ((sourceDir == toDir) && (sourName == toDirName)) {
		return true;
	}
	if (!QFile::exists(sourceDir)) {
		return false;
	}
	QDir createfile;
	bool exist = createfile.exists(strDecPath);
	if (exist) {
		if (coverFileIfExist) {
			createfile.remove(strDecPath);
		}
	}
	else
	{
		CreateDir(toDir);
	}

	QFileInfo fileInfo(toDir);
	if (!fileInfo.isDir())
	{
		return false;
	}


	if (!QFile::copy(strSPath, strDecPath))
	{
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		//MessageBox(NULL,(LPTSTR)lpMsgBuf, L"系统错误", MB_OK | MB_ICONSTOP);
		QString  strError = "";
		//MessageBox(NULL,(LPTSTR)lpMsgBuf, L"系统错误", MB_OK | MB_ICONSTOP);
		strError = QObject::tr("拷贝文件从：%1\r\n到%2时\r\n出现错误错误信息:%3").arg(strSPath).arg(strDecPath).arg(QString::fromUtf16((ushort*)lpMsgBuf));
		WriteLog(strError);
		LocalFree(lpMsgBuf);
		return false;
	}
	return true;
}

bool QtLogger::copyFileToFolder(QString srcFileName, QString desFilePathName, bool coverFileIfExist, bool bDelSrcFile)
{
	desFilePathName.replace("\\", "/");
	if (srcFileName == desFilePathName) {
		return true;
	}
	if (!QFile::exists(srcFileName)) {
		return false;
	}

	CreateDir(desFilePathName);// 校验目的文件夹路径是否存在，如果 不存在，创建

	QFileInfo srcFinfo(srcFileName);
	QFile srcFile(srcFileName);

	QString desFileName = desFilePathName + "/" + srcFinfo.fileName();// 目的文件全路径
	QFile desFinfo(desFileName);
	if (desFinfo.exists() == true)
	{
		if (coverFileIfExist)
		{
			desFinfo.remove();
		}
	}
	QFile::copy(srcFileName, desFileName);
	if (bDelSrcFile)//! 删除原文件
	{
		srcFile.remove();
	}

	return true;
}

bool QtLogger::CreateDir(QString dirName)
{
	dirName.replace('\\', '/');
	QStringList dirNameArray = dirName.split('/');
	int nameSize = dirNameArray.size();
	for (int i = 1; i < nameSize + 1; i++)
	{
		QString iBefAllDirStr = "";
		for (int j = 0; j < i; j++)
		{
			iBefAllDirStr += QString(dirNameArray.at(j) + '/');
		}

		//QString sCurNowDirName = dirNameArray.at(0);
		QDir diri(iBefAllDirStr);
		if (diri.exists() == false)
		{
			diri.mkdir(iBefAllDirStr);
		}
	}
	return true;
}

bool QtLogger::FolderCopy(const QString& source, const QString& destination, bool override)
{
	if (source == destination)
		return true;

	QDir directory(source);
	if (!directory.exists())
	{
		return false;
	}

	QDir directory1(destination);
	if (!directory1.exists())
	{
		CreateDir(destination);
	}


	QString srcPath = QDir::toNativeSeparators(source);
	if (!srcPath.endsWith(QDir::separator()))
		srcPath += QDir::separator();
	QString dstPath = QDir::toNativeSeparators(destination);
	if (!dstPath.endsWith(QDir::separator()))
		dstPath += QDir::separator();


	bool error = false;
	QStringList fileNames = directory.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
	for (QStringList::size_type i = 0; i != fileNames.size(); ++i)
	{
		QString fileName = fileNames.at(i);
		QString srcFilePath = srcPath + fileName;
		QString dstFilePath = dstPath + fileName;
		QFileInfo fileInfo(srcFilePath);
		if (fileInfo.isFile() || fileInfo.isSymLink())
		{
			if (override)
			{
				QFileInfo dstInfo(dstFilePath);
				if (dstInfo.isFile())
				{
					if (!QFile::remove(dstFilePath))
					{
						return false;
					}
				}
				QFile::setPermissions(dstFilePath, QFile::WriteOwner);
			}
			if (!QFile::copy(srcFilePath, dstFilePath))
			{
				return false;
			}
		}
		else if (fileInfo.isDir())
		{
			QDir dstDir(dstFilePath);
			dstDir.mkpath(dstFilePath);
			if (!FolderCopy(srcFilePath, dstFilePath, override))
			{
				error = true;
			}
		}
	}


	return !error;
}

bool QtLogger::DelDir(const QString& path)
{
	if (path.isEmpty()) {
		return false;
	}
	QDir dir(path);
	if (!dir.exists()) {
		return true;
	}
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤
	QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息
	foreach(QFileInfo file, fileList) { //遍历文件信息
		if (file.isFile()) { // 是文件，删除
			file.dir().remove(file.fileName());
		}
		else { // 递归删除
			DelDir(file.absoluteFilePath());
		}
	}
	return dir.rmpath(dir.absolutePath()); // 删除文件夹
}

bool QtLogger::FolderCopy2(const QString& srcFilePath, const QString& tgtFilePath)
{
	QString src = QDir::cleanPath(srcFilePath);
	QString tgt = QDir::cleanPath(tgtFilePath);

	// 防止把目录拷到自身的子目录，避免无限递归
	if (tgt.startsWith(src + QDir::separator()))
		return false;

	QFileInfo srcInfo(src);
	if (!srcInfo.exists())
		return false;

	if (srcInfo.isDir()) {
		QDir targetDir;
		// 使用 mkpath 创建目标路径（包括中间目录）
		if (!targetDir.mkpath(tgt))
			return false;

		QDir sourceDir(src);
		QStringList entries = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
		for (const QString& entry : entries) {
			const QString newSrc = sourceDir.filePath(entry);
			const QString newTgt = QDir(tgt).filePath(entry);
			if (!FolderCopy2(newSrc, newTgt))
				return false;
		}
	}
	else {
		// 如果目标已经存在，先删除或覆盖（可按需调整）
		QFileInfo tgtInfo(tgt);
		if (tgtInfo.exists()) {
			if (tgtInfo.isDir())
				return false; // 目标为目录但源为文件，认为失败（可改为删除目录）
			if (!QFile::remove(tgt))
				return false;
		}
		else {
			// 确保目标父目录存在
			QDir parentDir = QFileInfo(tgt).dir();
			if (!parentDir.exists()) {
				if (!parentDir.mkpath("."))
					return false;
			}
		}

		if (!QFile::copy(src, tgt))
			return false;
	}
	return true;
}

bool QtLogger::IsDirectoryExists(QString fullpath)
{
	QFileInfo dir(fullpath);
	if (dir.isDir())
	{
		return true;
	}
	return false;
}

bool QtLogger::isFileExist(QString fullFileName)
{
	QFile fileInfo(fullFileName);
	if (fileInfo.exists())
	{
		return true;
	}
	return false;
}

void QtLogger::ThreadSleep(int mSecond)
{
	QThread::msleep(static_cast<quint64>(mSecond));
}

void QtLogger::TimerDelay(int mSecond)
{

	QElapsedTimer et;
	et.start();
	while (et.elapsed() < mSecond)
	{
		QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
		//Sleep(uint(10));
	}
}

bool QtLogger::getFileNameInfo(QString path, QList<QFileInfo>& fileInfo, QString strSuffix)
{
	QDir dir(path);
	if (!dir.exists())		return false;		//文件不存在
	QStringList filter;
	//只查找.txt后缀的文件
	QString  str = "*" + strSuffix;
	filter << str;
	dir.setNameFilters(filter);
	QList<QFileInfo> FileInfo(dir.entryInfoList(filter, QDir::AllEntries | QDir::Readable, QDir::Name));
	fileInfo = FileInfo;

	return true;
}

bool QtLogger::RemoveFolderContent(const QString& folderDir)
{
	QDir dir(folderDir);
	QFileInfoList fileList;
	QFileInfo curFile;
	if (!dir.exists()) { return false; }//文件不存，则返回false
	fileList = dir.entryInfoList(QDir::Dirs | QDir::Files
		| QDir::Readable | QDir::Writable
		| QDir::Hidden | QDir::NoDotAndDotDot
		, QDir::Name);
	while (fileList.size() > 0)
	{
		int infoNum = fileList.size();
		for (int i = infoNum - 1; i >= 0; i--)
		{
			curFile = fileList[i];
			if (curFile.isFile())//如果是文件，删除文件
			{
				QFile fileTemp(curFile.filePath());
				fileTemp.remove();
				fileList.removeAt(i);
			}
			if (curFile.isDir())//如果是文件夹
			{
				QDir dirTemp(curFile.filePath());
				QFileInfoList fileList1 = dirTemp.entryInfoList(QDir::Dirs | QDir::Files
					| QDir::Readable | QDir::Writable
					| QDir::Hidden | QDir::NoDotAndDotDot
					, QDir::Name);
				if (fileList1.size() == 0)//下层没有文件或文件夹
				{
					dirTemp.rmdir(".");
					fileList.removeAt(i);
				}
				else//下层有文件夹或文件
				{
					for (int j = 0; j < fileList1.size(); j++)
					{
						if (!(fileList.contains(fileList1[j])))

							fileList.append(fileList1[j]);
					}
				}
			}
		}
	}
	dir.removeRecursively();			//这句话会把最最外层的目录给删除
	return true;
}

bool QtLogger::clearAllDirFile(const QString& path)
{
	if (QtLogger::IsDirectoryExists(path))
	{
		QDir dir(path);
		if (!dir.exists())
		{
			WriteLog(QObject::tr("文件路径是空"));
			return false;
		}
		QList<QFileInfo> fileInfo = dir.entryInfoList(QDir::AllEntries | QDir::Readable, QDir::Name);
		//如果找到则进行删除操作
		for (int i = 0; i < fileInfo.count(); i++)
		{
			QString strFileName = fileInfo.at(i).fileName();
			if (strFileName == "." || strFileName == "..")
				continue;

			QString  filePath = QObject::tr("%1/%2").arg(path).arg(strFileName);
			if (!QFile::remove(filePath))
				return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}

bool QtLogger::RenameFile(QString newPathName, QString oldPathName)
{
	QFileInfo file(oldPathName);
	if (!file.exists()) {
		QString str = QObject::tr("file not exist:%1").arg(oldPathName);
		WriteLog(str);
		return  false;
	}
	bool ok = QFile::rename(oldPathName, newPathName);
	return ok;
}

bool QtLogger::clearAllFiles(QString path, QString strSuffix)
{

	if (QtLogger::IsDirectoryExists(path))
	{
		QList<QFileInfo> fileInfo;		//文件信息
		if (getFileNameInfo(path, fileInfo, strSuffix))
		{
			//DeleteDirectory(path);
			//如果找到则进行删除操作
			for (int i = 0; i < fileInfo.count(); i++)
			{
				QString strFileName = fileInfo.at(i).fileName();
				if (strFileName == "." || strFileName == "..")
					continue;

				QString  filePath = QObject::tr("%1/%2").arg(path).arg(strFileName);
				if (!QFile::remove(filePath))
					return false;
			}

		}
	}
	else
	{
		return false;
	}

	return true;
}

bool QtLogger::DeleteDirectory(const QString path)
{
	if (path.isEmpty())
		return false;
	QDir dir(path);
	if (!dir.exists())
		return true;
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	QFileInfoList fileList = dir.entryInfoList();
	foreach(QFileInfo fi, fileList)
	{
		if (fi.isFile())
			fi.dir().remove(fi.fileName());
		else
			DeleteDirectory(fi.absoluteFilePath());
	}
	return dir.rmpath(dir.absolutePath());
}

bool QtLogger::isDigitStr(QString src)
{
	QByteArray ba = src.toLatin1();//QString 转换为 char*
	const char* s = ba.data();
	while (*s && *s >= '0' && *s <= '9') s++;
	if (*s)
	{ //不是纯数字
		return false;
	}
	return true;
}

int QtLogger::findSubstrNum(QChar substr, QString strv)
{
	QString str = strv;
	int size = str.length();
	QString strTemp;
	int count = 0;
	for (int i = 0; i < size; i++) {
		strTemp = str.section(substr, i, i);
		if (strTemp.isEmpty())
		{
			break;
		}
		count++;
	}
	return count - 1;
}

QString QtLogger::GetRightSpitPath(QString strPeizhiPath)
{
	QString strTemp = strPeizhiPath;
	strTemp = strTemp.replace("&", "");
	strTemp = strTemp.replace("$", "");
	return strTemp;
}

bool QtLogger::InsertTextToFile(const QString pszScrFilePath, const QString strCatText, const QString InsertMark, int InsertMode)
{
	InsertMode = 0;
	return false;
}

bool  QtLogger::GetSubTextByFile(const QString pszScrFilePath, QString& pszOutText, QString StartMark, QString EndMark, int Contain)
{
	if (pszScrFilePath.isEmpty() || StartMark.isEmpty() || EndMark.isEmpty())
	{
		WriteLog(QObject::tr("获取文件pszScrFilePath:%1中内容时，\r\n传入数据为空StartMark: %2, EndMark: %3").arg(pszScrFilePath).arg(StartMark).arg(EndMark));
		return false;
	}
	QString filePath = pszScrFilePath;				//源文件所在路径
	QFileInfo  tempFind(filePath);
	if (!tempFind.isFile())
	{
		WriteLog(QObject::tr("文件不存在%1").arg(filePath));
		return false;
	}

	StartMark = StartMark.toUpper();
	StartMark = StartMark.replace(" ", "");
	EndMark = EndMark.toUpper();
	EndMark = EndMark.replace(" ", "");
	QFile myfile(filePath);				//用于读原始文件
	if (myfile.open(QIODevice::ReadOnly | QIODevice::Text))
	{

		bool mark = false;
		while (!myfile.atEnd())
		{
			QString strTemp;
			QString strLine = "";
			QByteArray line = myfile.readLine();
			QTextCodec* tc1 = QTextCodec::codecForName("GBK");
			strLine = tc1->toUnicode(line);
			strTemp = strLine;
			strTemp = strTemp.toUpper();
			strTemp = strTemp.replace(" ", "");
			if (Contain == 0)
			{
				if ((strTemp.contains(StartMark))
					&& mark == false)
				{
					//pszOutText = strTemp;
					mark = true;
					continue;
				}
				if (mark)
				{
					if (strTemp.contains(EndMark))
					{
						break;
					}
					pszOutText += strLine;
				}
			}
			else if (Contain == 1)
			{
				if ((strTemp.contains(StartMark))
					&& mark == false)
				{
					pszOutText += strLine;
					mark = true;
					continue;
				}
				if (mark)
				{
					if (strTemp.contains(EndMark))
					{
						pszOutText += strLine;
						break;
					}
					pszOutText += strLine;
				}
			}
			else if (Contain == 2)
			{
				if ((strTemp.contains(StartMark))
					&& mark == false)
				{
					pszOutText += strLine;
					mark = true;
					continue;
				}
				if (mark)
				{
					if (strTemp.contains(EndMark))
					{
						//pszOutText += strLine;
						break;
					}
					pszOutText += strLine;
				}
			}
		}
		myfile.close();
	}
	else
	{
		QString strError = "";
		strError = QObject::tr("获取文本时，打开 %1 文件失败").arg(filePath);
		QtLogger::WriteLog(strError);
		return false;
	}
	return true;
}

QString QtLogger::StrChineseTranscoding(QString  SourStr)
{
	QString  strTemp = "";
	QByteArray bytes = SourStr.toLatin1();
	strTemp.prepend(bytes);
	return strTemp;
}

bool QtLogger::connectSqlliteDB()
{
	QString logFileDir = QCoreApplication::applicationDirPath() + "/DataBase/Sqlite/";
	QString dataBaseName = logFileDir + "EATMDataBase.db";
	QFileInfo fi(dataBaseName);
	if (fi.isFile())
	{

	}
	return true;
}

void QtLogger::writelogInfoTodb()
{

}

bool QtLogger::ShowThreadId(QString strPosition)
{
#ifdef QT_NO_DEBUG
	//relese模式

#else
	//DEBUG模式
	QString threadText = QObject::tr("%1 线程ID: @0x%2").arg(strPosition).arg(quintptr(QThread::currentThreadId()), 16, 16, QLatin1Char('0'));
	WriteLog(threadText);
#endif

	return true;
}

bool QtLogger::ExectCmdPing(QString strIP)
{
	// 验证IP地址格式，防止命令注入
	static QRegularExpression re("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
	if (!re.match(strIP.trimmed()).hasMatch()) {
		WriteLog(QObject::tr("ExectCmdPing: 无效的IP地址格式: %1").arg(strIP), enLogType::WARNING);
		return false;
	}

	QProcess exc;
	QTextCodec* codec = QTextCodec::codecForName("GBK");
	QStringList args;
	args << strIP << "-n" << "2" << "-w" << "4000";
	exc.start("ping", args);
	exc.waitForFinished(-1);
	QString outstr = codec->toUnicode(exc.readAll());

	if ((-1 != outstr.indexOf(QObject::tr("往返行程的估计时间"))) || (-1 != outstr.indexOf("Packets: Sent = 4, Received = 4, Lost = 0")))
	{
		return true;
	}
	return false;
}

QString QtLogger::GetExeLastModifyTime()
{
	QStringList args = QCoreApplication::instance()->arguments();
	QString file = args.join(" ");
	int start = file.lastIndexOf('\\');
	file = file.mid(start + 1);

	QString logFileDir = QCoreApplication::applicationDirPath() + "/" + file;

	QFileInfo info(logFileDir);
	//获取程序生成时间
	QString  strTime = "";		//最新修改过的时间
	//strTime = info.birthTime().toString("yyyy-MM-dd hh:mm:ss");				//最新修改过的时间
	//strTime = info.created().toString("yyyy-MM-dd hh:mm:ss");					//最新修改过的时间
	strTime = info.lastModified().toString("yyyy-MM-dd hh:mm:ss");				//最后的编译时间

	return strTime;
}

bool QtLogger::CheckWriteLogSpaceTime(int spaceTime)
{
	QString strCurrentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	QString  strTime = GetExeLastModifyTime();
	int nDays = DaysBetween2Date(strTime, strCurrentTime);
	if (nDays <= spaceTime)
	{
		//间隔时间小于计划的时间返回true，继续打印日志
		return true;
	}

	return false;
}

int QtLogger::DaysBetween2Date(QString date1, QString date2)
{
	int year1, month1, day1;
	int year2, month2, day2;
	if (!StringToDate(date1, year1, month1, day1) || !StringToDate(date2, year2, month2, day2))                           //调用截取函数
	{
		return -1;                                          //如果截取信息失败，那么将返回-1
	}
	if (year1 == year2 && month1 == month2)                 //如果年月相同，则返回相信日期相减数据
	{
		return day1 > day2 ? day1 - day2 : day2 - day1;
	}
	else if (year1 == year2)                                //如果年份相同，
	{
		int d1, d2;
		d1 = DayInYear(year1, month1, day1);               //调用月份年年份函数来获得数据
		d2 = DayInYear(year2, month2, day2);
		return d1 > d2 ? d1 - d2 : d2 - d1;
	}
	else                                                   //如果年份不同，这重新计算
	{
		if (year1 > year2)                                  //如果前方数据大于后方，这调换这两数据
		{
			std::swap(year1, year2);
			std::swap(month1, month2);
			std::swap(day1, day2);
		}
		int d1, d2, d3;
		if (IsLeap(year1))
			d1 = 366 - DayInYear(year1, month1, day1);
		else
			d1 = 365 - DayInYear(year1, month1, day1);
		d2 = DayInYear(year2, month2, day2);
		d3 = 0;
		for (int year = year1 + 1; year < year2; year++)
		{
			if (IsLeap(year))
				d3 += 366;
			else
				d3 += 365;
		}
		return d1 + d2 + d3;
	}
}

bool QtLogger::StringToDate(QString date, int& year, int& month, int& day)
{
	year = date.mid(0, 4).toInt();                                                   //数据截取
	month = date.mid(5, 2).toInt();                                                  //数据截取
	day = date.mid(8, 2).toInt();                                                    //数据截取

	int DAY[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };                              //初始化12个月份的数据
	if (IsLeap(year))                                                                //如果是闰年，那么将2月的数据更新为29天
	{
		DAY[1] = 29;
	}
	return year >= 0 && month <= 12 && month > 0 && day <= DAY[month - 1] && day > 0;         //年天算法，判断数据解析是否符合日期规格
}

bool QtLogger::IsLeap(int year)
{
	return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

int QtLogger::DayInYear(int year, int month, int day)
{
	int DAY[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	if (IsLeap(year))
		DAY[1] = 29;
	for (int i = 0; i < month - 1; ++i)
	{
		day += DAY[i];
	}
	return day;
}

bool QtLogger::isRUNyear(int year)
{
	if ((year % 4 == 0) && (year % 100 != 0) || (year % 400 == 0))
	{
		return true;
	}
	return false;
}

QDateTime QtLogger::changeStrToCTime(QString strTime)
{
	QString strBuffer = strTime;
	QDateTime time;
	int start = strBuffer.indexOf(' ');

	if (start == -1)
	{
		//2010-07-2
		QString strDay = strBuffer.section('-', 2, 2);
		if (strDay.size() < 2)
		{
			strDay = QObject::tr("%1%2").arg(0).arg(strDay);
		}
		QString strTemp = strBuffer.left(start);
		int second = strTemp.lastIndexOf('-');
		QString strLeft = strTemp.left(second + 1);
		strBuffer = strLeft + strDay + " 00:00:00";
	}
	else
	{
		QString strRight = strBuffer.mid(start);

		QString strBuffer2 = strBuffer.left(start);
		QString strDay = strBuffer2.section('-', 2, 2);
		if (strDay.size() < 2)
		{
			strDay = QObject::tr("%1%2").arg(0).arg(strDay);
		}
		QString strTemp = strBuffer2.left(start);
		int second = strTemp.lastIndexOf('-');
		QString strLeft = strTemp.left(second + 1);
		strBuffer = strLeft + strDay + strRight;
	}

	time = QDateTime::fromString(strBuffer, "yyyy-MM-dd hh:mm:ss");
	return time;
}

bool QtLogger::isInSameDy(QString strTime, QDateTime currentTime)
{
	QString strCString = strTime;
	strCString.replace('/', '-');
	strCString.replace('T', ' ');

	//比较年
	if (strCString.section('-', 0, 0) != currentTime.date().year())
		return false;

	//比较月
	if (strCString.section('-', 1, 1) != currentTime.date().month())
		return false;

	if (strCString.indexOf(' ') == -1)
	{
		// 只有日期没有时间，默认为 00:00:00
		strCString += " 00:00:00";
	}

	QDateTime strDateTime = QDateTime::fromString(strCString, "yyyy-MM-dd hh:mm:ss");

	return currentTime.date() == strDateTime.date();
}

void QtLogger::LastTenDays(QDateTime currentTime, QMap <int, QString>& m_day)
{
	m_day.clear();
	QString t = currentTime.toString("yyyy-MM-dd-hh-mm-ss-ddd");
	int currentDay = t.section('-', 2, 2).toInt(); //获得当前日期
	int currentMonth = t.section('-', 1, 1).toInt(); //获取当前月份
	int currentYear = t.section('-', 0, 0).toInt(); //获取当前年份
	QString strTemp = QObject::tr("");
	if (currentDay > 9)
	{
		QString sCurMonth = QString::number(currentMonth);
		if (sCurMonth.size() < 2)
		{
			sCurMonth = QObject::tr("%1%2").arg(0).arg(sCurMonth);
		}
		for (int i = 1; i <= 10; i++)
		{
			strTemp = QObject::tr("%1-%2-%3").arg(currentYear).arg(sCurMonth).arg(currentDay--);
			QDateTime tTempTime = changeStrToCTime(strTemp);
			m_day[i] = tTempTime.toString("yyyy-MM-dd hh:mm:ss");

		}
	}
	else
	{
		int firstDay = 10 - currentDay;
		int LocalMonthDay = currentDay;
		QString sCurMonth = QString::number(currentMonth);
		if (sCurMonth.size() < 2)
		{
			sCurMonth = QObject::tr("%1%2").arg(0).arg(sCurMonth);
		}
		for (int i = 1; i <= currentDay; i++)
		{
			strTemp = QObject::tr("%1-%2-%3").arg(currentYear).arg(sCurMonth).arg(LocalMonthDay--);
			QDateTime tTempTime = changeStrToCTime(strTemp);
			m_day[i] = tTempTime.toString("yyyy-MM-dd hh:mm:ss");
		}
		if (currentMonth == 1)
		{
			currentYear = currentYear - 1;
			currentMonth = 12;
			int iTempDay = 31;
			for (int i = 1; i <= firstDay; i++)
			{
				strTemp = QObject::tr("%1-%2-%3").arg(currentYear).arg(currentMonth).arg(iTempDay--);
				QDateTime tTempTime = changeStrToCTime(strTemp);
				m_day[i + currentDay] = tTempTime.toString("yyyy-MM-dd hh:mm:ss");
			}
		}
		else if (currentMonth == 2 || currentMonth == 4 || currentMonth == 6 || currentMonth == 9 || currentMonth == 11 || currentMonth == 8)
		{
			int iTempDay = 31;
			currentMonth = currentMonth - 1;
			QString sCurMonth = QString::number(currentMonth);
			if (sCurMonth.size() < 2)
			{
				sCurMonth = QObject::tr("%1%2").arg(0).arg(sCurMonth);
			}
			for (int i = 1; i <= firstDay; i++)
			{
				strTemp = QObject::tr("%1-%2-%3").arg(currentYear).arg(sCurMonth).arg(iTempDay--);
				QDateTime tTempTime = changeStrToCTime(strTemp);
				m_day[i + currentDay] = tTempTime.toString("yyyy-MM-dd hh:mm:ss");
			}
		}
		else if (currentMonth == 5 || currentMonth == 7 || currentMonth == 10 || currentMonth == 12)
		{
			currentMonth = currentMonth - 1;
			QString sCurMonth = QString::number(currentMonth);
			if (sCurMonth.size() < 2)
			{
				sCurMonth = QObject::tr("%1%2").arg(0).arg(sCurMonth);
			}
			int iTempDay = 30;
			for (int i = 1; i <= firstDay; i++)
			{
				strTemp = QObject::tr("%1-%2-%3").arg(currentYear).arg(sCurMonth).arg(iTempDay--);
				QDateTime tTempTime = changeStrToCTime(strTemp);
				m_day[i + currentDay] = tTempTime.toString("yyyy-MM-dd hh:mm:ss");
			}
		}
		else
		{
			currentMonth = currentMonth - 1;
			QString sCurMonth = QString::number(currentMonth);
			if (sCurMonth.size() < 2)
			{
				sCurMonth = QObject::tr("%1%2").arg(0).arg(sCurMonth);
			}
			if (isRUNyear(currentYear))
			{
				int iTempDay = 29;
				for (int i = 1; i <= firstDay; i++)
				{
					strTemp = QObject::tr("%1-%2-%3").arg(currentYear).arg(sCurMonth).arg(iTempDay--);
					QDateTime tTempTime = changeStrToCTime(strTemp);
					m_day[i + currentDay] = tTempTime.toString("yyyy-MM-dd hh:mm:ss");
				}
			}
			else
			{
				int iTempDay = 28;
				for (int i = 1; i <= firstDay; i++)
				{
					strTemp = QObject::tr("%1-%2-%3").arg(currentYear).arg(sCurMonth).arg(iTempDay--);
					QDateTime tTempTime = changeStrToCTime(strTemp);
					m_day[i + currentDay] = tTempTime.toString("yyyy-MM-dd hh:mm:ss");
				}
			}
		}
	}
}

bool QtLogger::CloseMyself()
{
	QStringList args = QCoreApplication::instance()->arguments();
	QString file = args.join(" ");
	int start = file.lastIndexOf('\\');
	file = file.mid(start + 1);
	QProcess p;
	p.start("taskkill", QStringList() << "/im" << file << "/f");
	p.waitForFinished(-1);
	p.close();
	return true;
}

bool QtLogger::CloseApplicationProgram(QString strExename)
{
	if (strExename.isEmpty())  return false;

	// 验证进程名，防止命令注入（只允许字母、数字、下划线、点、横杠）
	static QRegularExpression re("^[a-zA-Z0-9_\\-.]+\\.exe$", QRegularExpression::CaseInsensitiveOption);
	if (!re.match(strExename.trimmed()).hasMatch()) {
		WriteLog(QObject::tr("CloseApplicationProgram: 无效的进程名: %1").arg(strExename), enLogType::WARNING);
		return false;
	}

	QProcess p;
	p.start("taskkill", QStringList() << "/im" << strExename << "/f");
	p.waitForFinished(-1);
	p.close();
	return true;
}
