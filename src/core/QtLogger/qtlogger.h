#pragma once

#include <QDir>
#include <QTimer>
#include <QThread>
#include <QVector>
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QObject>
#include <QMutex>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QReadWriteLock>
#include <QTextCodec>
#include <QProcess>

#include <QtCore/qglobal.h>

#ifdef QTLOGGER_LIBRARY
#  define QTLOGGER_EXPORT Q_DECL_EXPORT
#else
#  define QTLOGGER_EXPORT Q_DECL_IMPORT
#endif


//日志类型
enum enLogType
{
	OPERATER = 1,			//操作
	EDEBUG,					//调试
	WARNING,				//警告
	SERIOUS,				//严重
	DEADLY,					//致命
};

class ThreadLog : public QObject
{
	Q_OBJECT
public:
	ThreadLog();
	~ThreadLog();
private slots:
	/**
	 * @brief writeLogThread 写入日志线程
	 * @param logType 日志类型
	 * @param strLogInfo 日志信息
	 * @param logPriority 日志优先级
	 * @return void
	 */
	void writeLogThread(int logType, QString strLogInfo, int logPriority = 0);
};


class QTLOGGER_EXPORT QtLogger : public QObject
{
	Q_OBJECT
public:
	/**
	 * @brief initSignalThread 初始化信号线程
	 * @param parent 父对象
	 * @return void
	 */
	void initSignalThread();

signals:
	/**
	 * @brief writeLogSignal 写入日志信号
	 * @param logType 日志类型
	 * @param strLogInfo 日志信息
	 * @param logPriority 日志优先级
	 * @return void
	 */
	void writeLogSignal(int logType, QString strLogInfo, int logPriority = 0);

public:
	QtLogger(QObject* parent = nullptr);
	~QtLogger();

	/**
	 * @brief setLogDir 设置日志目录
	 * @return void
	 */
	static QString getFileInfo() { return QObject::tr("%1 (%2) - <%3> \n").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__); };

	/**
	 * @brief setLogDir 写入日志实际处理函数
	 * @param logType 日志类型
	 * @param strLogInfo 日志信息
	 * @param logPriority 日志优先级
	 * @return void
	 */
	void WriteReallyLog(int logType, QString strLogInfo, int logPriority = 0);

	/**
	 * @brief strSrc 源字符串
	 * @param iLen 小数点后长度
	 * @return QString 处理后的字符串
	 */ 
	static QString GetDecimalLen(QString strSrc, int iLen);

public:
	QString logFileDir;
	QString logFileName;
	QString logFilePath;
	QTextStream* logStream;
	QFile* logFile;
	//volatile bool stopped;
	QMutex mutex;
	ThreadLog* m_signalThreadLog;
	QThread* m_reallThread;
	QMutex sleepMutex;

public:
	/**
	 * @brief WriteLog 写入日志函数
	 * @param strLogInfo 日志信息
	 * @param logType 日志类型
	 * @param logPriority 日志优先级
	 * @return void
	 */
	static void WriteLog(QString strLogInfo, int logType = enLogType::EDEBUG, int logPriority = 0);

	/**
     * @brief 创建文件夹，里面有递归创建操作
     * @param dirName 源文件全路径，比如  "F:/tx/des/desd/wwxx" 如果已经存在，则什么也不做，如果原先不存在，则创建
	 * @return 成功返回true，失败返回false
     */
	static bool CreateDir(QString dirName);

	/**
     * @brief 拷贝文件夹到目的文件夹
     * @param source 源文件夹全路径，比如  "F:/tx" ,"F:/txd/des/desd"
     * @param destination 要COPY到的目的路径 比如 "F:/tx/des/desd"
     * @param override 如果目的文件存在，比如 "F:/txd/des/desd" 存在，是否覆盖，true表示覆盖
	 * @return 成功返回true，失败返回false
     */
	static bool FolderCopy(const QString& source, const QString& destination, bool override = true);

	/**
	 * @brief 复制文件到指定文件夹
	 * @param srcFileName 源文件全路径，比如  "F:/tx/des/desd/wwxx.txt"
	 * @param desFilePathName 目标文件夹全路径，比如  "F:/tx/des/desd/"
	 * @param coverFileIfExist 如果目标文件夹下存在同名文件，是否覆盖，true表示覆盖
	 * @param bDelSrcFile 是否删除源文件,true表示删除
	 * @return 成功返回true，失败返回false
	 */
	static bool copyFileToFolder(QString srcFileName, QString desFilePathName, bool coverFileIfExist, bool bDelSrcFile);

	/**
	 * @brief 复制文件到指定文件夹并重命名
	 * @param sourceDir 源文件夹路径
	 * @param toDir 目标文件夹路径
	 * @param sourName 源文件名称
	 * @param toDirName 目标文件名称
	 * @param coverFileIfExist 如果目标文件夹下存在同名文件，是否覆盖，true表示覆盖
	 * @return 成功返回true，失败返回false
	 */
	static bool FileCopy(QString sourceDir, QString toDir, QString sourName, QString toDirName, bool coverFileIfExist = true);

	/**
	 * @brief 递归拷贝文件夹
	 * @param srcFilePath 源文件夹路径
	 * @param tgtFilePath 目标文件夹路径
	 * @return 成功返回true，失败返回false
	 */
	static bool FolderCopy2(const QString& srcFilePath, const QString& tgtFilePath);

	/**
	 * @brief IsDirectoryExists 判断指定目录是否存在
	 * @param fullpath 路径
	 * @return 成功返回true，失败返回false
	 */
	static bool IsDirectoryExists(QString fullpath);

	/**
	 * @brief isFileExist 判断指定文件是否存在
	 * @param fullFileName 文件全路径
	 * @return 成功返回true，失败返回false
	 */ 
	static bool isFileExist(QString fullFileName);

	/**
	 * @brief DelDir 删除目录
	 * @param path 目录路径
	 * @return 成功返回true，失败返回false
	 */
	static bool DelDir(const QString& path);

	/**
	 * @brief ThreadSleep 线程睡眠函数
	 * @param mSecond 睡眠秒数
	 * @return void
	 */
	static void ThreadSleep(int mSecond);

	/**
	 * @brief TimerDelay 定时器延时函数
	 * @param mSecond 延时秒数
	 * @return void
	 */
	static void TimerDelay(int mSecond);

	/**
	 * @brief getFileNameInfo 获取指定路径下指定后缀的所有文件信息
	 * @param path 指定路径
	 * @param fileInfo 获取的文件信息
	 * @param strSuffix 要获取的文件后缀名
	 * @return 成功返回true，失败返回false
	 */
	static bool getFileNameInfo(QString path, QList<QFileInfo>& fileInfo, QString strSuffix);

	/**
	 * @brief isDigitStr 判断字符串是否全部为数字
	 * @param src 字符串
	 * @return 成功返回true，失败返回false
	 */
	static bool isDigitStr(QString src);

	/**
	 * @brief findSubstrNum 找字符串中字符的个数
	 * @param substr 字符 & %￥#等
	 * @param strv 字符串
	 * @return 字符个数
	 */
	static int findSubstrNum(QChar substr, QString strv);

	/**
	 * @brief clearAllFiles 删除指定路径下指定后缀的所有文件
	 * @param path 指定路径
	 * @param strSuffix 要删除的文件后缀名
	 * @return 成功返回true，失败返回false
	 */
	static bool clearAllFiles(QString path, QString strSuffix);

	/**
	 * @brief clearAllDirFile 删除指定路径下的所有文件
	 * @param path 指定路径
	 * @return 成功返回true，失败返回false
	 */
	static bool clearAllDirFile(const QString& path);

	/**
	 * @brief DeleteDirectory 删除指定的文件或者文件夹
	 * @param path 指定路径
	 * @return 成功返回true，失败返回false
	 */
	static bool DeleteDirectory(const QString path);

	/**
	 * @brief RenameFile 修改指定文件名称
	 * @param newName 新文件名称
	 * @param oldName 旧文件名称
	 * @return 成功返回true，失败返回false
	 */
	static bool RenameFile(QString newName, QString oldName);

	/**
	 * @brief RemoveFolderContent 删除文件夹下的所有内容，但不删除该文件夹
	 * @param folderDir 文件夹路径
	 * @return 成功返回true，失败返回false
	 */
	static bool RemoveFolderContent(const QString& folderDir);

	/**
	 * @brief GetRightSpitPath 获取配置文件路径最后的文件名
	 * @param strPeizhiPath 配置文件路径
	 * @return 文件名
	 */
	static QString GetRightSpitPath(QString strPeizhiPath);

	/**
	 * @brief InsertTextToFile 将文本strCatText插入到文件pszScrFilePath中指定标记InsertMark的位置
	 * @param pszScrFilePath 文件路径
	 * @param strCatText 要插入的文本
	 * @param InsertMark 插入标记
	 * @param InsertMode 插入模式，1表示在标记后插入，2表示在标记前插入
	 * @return 成功返回true，失败返回false
	 */
	static bool InsertTextToFile(const QString pszScrFilePath, const QString strCatText, const QString InsertMark, int InsertMode = 1);

	/**
	 * @brief GetSubTextByFile 将文件pszScrFilePath首尾之间的内容添加到文本pszOutText中
	 * @param pszScrFilePath 文件路径
	 * @param pszOutText 输出的文本
	 * @param StartMark 开始标记
	 * @param EndMark 结束标记
	 * @param Contain 0,不包含首尾，1包含首尾，2，包含首，不包含尾
	 * @return 成功返回true，失败返回false
	 */
	static bool GetSubTextByFile(const QString pszScrFilePath, QString& pszOutText, QString StartMark, QString EndMark, int Contain = 0);

	/**
	 * @brief StrChineseTranscoding 解决配置文件中读取中文乱码
	 * @param SourStr 原始字符串
	 * @return 处理后的字符串
	 */
	static QString  StrChineseTranscoding(QString  SourStr);

	/**
	 * @brief isRUNyear 判断是否是闰年
	 * @param year 传入年份
	 * @return 是闰年返回true，否则返回false
	 * @note 平年28天、闰年29天
	 * @note 闰年判定：能被400整除。或者能被4整除但不能被100整除。其余的年份都为平年
	 */
	static bool isRUNyear(int year);

	/**
	 * @brief changeStrToCTime 字符串类型的时间转化为QDateTime 类型的时间处理函数
	 * @param strTime 字符串类型的时间 格式为yyyy-MM-dd 00:00:00
	 * @return QDateTime 类型的时间
	 */
	static QDateTime changeStrToCTime(QString strTime);

	/**
	 * @brief LastTenDays 根据当前时间取过去十天的日期的处理函数
	 * @param currentTime 传入时间
	 * @param m_day 存取的十天的凌晨时间
	 * @return void
	 * @note 存取的十天的凌晨时间，key为天数，value为对应的日期字符串，格式为yyyy-MM-dd 00:00:00
	 */
	static void LastTenDays(QDateTime currentTime, QMap <int, QString>& m_day);

	/**
	 * @brief isInSameDy 判断两个时间是否为同一天
	 * @param strTime 字符串类型的时间 格式为yyyy-MM-dd 00:00:00
	 * @param currentTime 传入时间
	 * @return 成功返回true，失败返回false
	 */
	static bool isInSameDy(QString strTime, QDateTime currentTime);

public:
	/*****************把日志写到sqlite数据库******************/

	/**
	 * @brief connectSqlliteDB 数据库的连接
	 * @return 成功返回true，失败返回false
	 */
	bool connectSqlliteDB();

	/**
	 * @brief writelogInfoTodb 把日志写入数据库的静态函数
	 * @return void
	 */
	static void writelogInfoTodb();

	/**
	 * @brief isIntDataInList 判断int数据是否在 list里面，是返回ture
	 * @param strNum 数据
	 * @param tempList 存储数据的list
	 * @return 成功返回true，失败返回false
	 */
	static bool ShowThreadId(QString strPosition);

	/**
	 * @brief ExectCmdPing 执行 ping 的指令
	 * @param strIP IP地址
	 * @return 成功返回true，失败返回false
	 */
	static bool ExectCmdPing(QString strIP);

	/**
	 * @brief GetExeLastModifyTime 获取exe最后的编译时间
	 * @return 编译时间字符串
	 */
	static QString GetExeLastModifyTime();

	/**
	 * @brief CheckWriteLogSpaceTime 检查写日志间隔时间
	 * @param spaceTime 间隔时间，单位秒
	 * @return 成功返回true，失败返回false
	 */
	static bool CheckWriteLogSpaceTime(int spaceTime);

	/**
	 * @brief DaysBetween2Date 计算两个日期之间相差的天数
	 * @param date1 日期1，格式为yyyy-MM-dd
	 * @param date2 日期2，格式为yyyy-MM-dd
	 * @return 相差的天数
	 */
	static int DaysBetween2Date(QString date1, QString date2);

	/**
	 * @brief StringToDate 字符串类型的时间转为 年 月 日
	 * @param date 字符串类型的时间，格式为yyyy-MM-dd
	 * @param year 年
	 * @param month 月
	 * @param day 日
	 * @return 成功返回true，失败返回false
     */
	static bool StringToDate(QString date, int& year, int& month, int& day);

	/**
	 * @brief IsLeap 判断是否是闰年
	 * @param year 年份
	 * @return 是闰年返回true，否则返回false
	 */
	static bool IsLeap(int year);

	/**
	 * @brief DayInYear 计算某一天是某一年的第几天
	 * @param year 年份
	 * @param month 月份
	 * @param day 日期
	 * @return 第几天
	 */
	static int DayInYear(int year, int month, int day);

	/**
	 * @brief CloseMyself 关闭自身应用程序
	 * @return 成功返回true，失败返回false
	 */
	static bool CloseMyself();

	/**
	 * @brief CloseApplicationProgram 关闭指定的应用程序
	 * @param strExename 应用程序名称，比如"notepad.exe"
	 * @return 成功返回true，失败返回false
	 */
	static bool CloseApplicationProgram(QString strExename);
};

#define WRITE_LOG(type, msg)(\
{\
    QString logType;\
    switch(type){\
        case QtDEBUGMsg:{logType = "[D]";}break;\
        case QtWarningMsg:{logType = "[W]";}break;\
        case QtCriticalMsg:{logType = "[C]";}break;\
        case QtFatalMsg:{logType = "[F]";}break;\
        default:{logType = "[I]";}break;}\
    QDateTime DT = QDateTime::currentDateTime();\
    QString DTStr = QString("[%1]").arg(DT.toString("yyyy-MM-dd hh:mm:ss"));\
    QString File = QString("[file:%1]").arg(__FILE__);\
    QString FuncStr = QString("[function:%1]").arg(__FUNCTION__);\
    QString Line = QString("[line:%1]").arg(__LINE__);\
    QString logMsg = QString("%1 %2 %3 %4 %5 %6").arg(DTStr).arg(logType).arg(File).arg(FuncStr).arg(Line).arg(msg);\
    qDebug("%s", logMsg.toLocal8Bit().data());\
    QtLogger::WriteLog(logMsg);\
})

#define LOGI(msg) (WRITE_LOG(QtInfoMsg,msg))
#define LOGD(msg) (WRITE_LOG(QtDEBUGMsg,msg))
#define LOGW(msg) (WRITE_LOG(QtWarningMsg,msg))
#define LOGC(msg) (WRITE_LOG(QtCriticalMsg,msg))
#define LOGF(msg) (WRITE_LOG(QtFatalMsg,msg))


