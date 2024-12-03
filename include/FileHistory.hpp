//
// Created by wuxianggujun on 2024/11/24.
//

#ifndef TINA_TOOL_BOX_FILE_HISTORY_HPP
#define TINA_TOOL_BOX_FILE_HISTORY_HPP

#include <QString>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <memory>

class FileHistory {
public:
    int id;
    QString fileName;
    QString filePath;
    QString fileType;
    qint64 fileSize;
    QDateTime modifiedDate;
    QDateTime createdAt;

    static void initTable(QSqlDatabase& db);
    static bool addRecord(QSqlDatabase& db, const FileHistory& record);
    static bool updateRecord(QSqlDatabase& db, const FileHistory& record);
    static bool deleteRecord(QSqlDatabase& db, const QString& filePath);
    static FileHistory getRecord(QSqlDatabase& db, const QString& filePath);
    static QVector<FileHistory> getAllRecords(QSqlDatabase& db, int limit = 10);
};

class FileHistoryManager {
public:
    static FileHistoryManager& getInstance();

    // 删除拷贝构造和赋值操作
    FileHistoryManager(const FileHistoryManager&) = delete;
    FileHistoryManager& operator=(const FileHistoryManager&) = delete;
    
    bool addFileHistory(const QString& filePath);
    bool updateFileHistory(const QString& filePath);
    bool deleteFileHistory(const QString& filePath);
    FileHistory getFileHistory(const QString& filePath);
    QVector<FileHistory> getRecentFiles(int limit = 10);

private:
    FileHistoryManager();
    ~FileHistoryManager();
    
    bool initDatabase();
    
    QString getDatabasePath() const;

    bool connectToDatabase();
    
    std::unique_ptr<QSqlDatabase> db_;
    bool isConnected_;
};




#endif //TINA_TOOL_BOX_FILE_HISTORY_HPP
