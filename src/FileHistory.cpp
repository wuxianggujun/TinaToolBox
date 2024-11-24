//
// Created by wuxianggujun on 2024/11/24.
//

#include "FileHistory.hpp"

#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include "spdlog/spdlog.h"

void FileHistory::initTable(QSqlDatabase& db) {
    QSqlQuery query(db);
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS file_history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            file_name VARCHAR(255) NOT NULL,
            file_path VARCHAR(1024) NOT NULL UNIQUE,
            file_type VARCHAR(50),
            file_size INTEGER,
            modified_date DATETIME,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )");
}

bool FileHistory::addRecord(QSqlDatabase& db, const FileHistory& record) {
    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO file_history (file_name, file_path, file_type, file_size, modified_date)
        VALUES (?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(record.fileName);
    query.addBindValue(record.filePath);
    query.addBindValue(record.fileType);
    query.addBindValue(record.fileSize);
    query.addBindValue(record.modifiedDate);
    
    return query.exec();
}

bool FileHistory::updateRecord(QSqlDatabase& db, const FileHistory& record) {
    QSqlQuery query(db);
    query.prepare(R"(
        UPDATE file_history 
        SET file_name = ?, file_type = ?, file_size = ?, modified_date = ?
        WHERE file_path = ?
    )");
    
    query.addBindValue(record.fileName);
    query.addBindValue(record.fileType);
    query.addBindValue(record.fileSize);
    query.addBindValue(record.modifiedDate);
    query.addBindValue(record.filePath);
    
    return query.exec();
}

bool FileHistory::deleteRecord(QSqlDatabase& db, const QString& filePath) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM file_history WHERE file_path = ?");
    query.addBindValue(filePath);
    return query.exec();
}

FileHistory FileHistory::getRecord(QSqlDatabase& db, const QString& filePath) {
    QSqlQuery query(db);
    query.prepare("SELECT * FROM file_history WHERE file_path = ?");
    query.addBindValue(filePath);
    
    FileHistory record;
    if (query.exec() && query.next()) {
        record.id = query.value("id").toInt();
        record.fileName = query.value("file_name").toString();
        record.filePath = query.value("file_path").toString();
        record.fileType = query.value("file_type").toString();
        record.fileSize = query.value("file_size").toLongLong();
        record.modifiedDate = query.value("modified_date").toDateTime();
        record.createdAt = query.value("created_at").toDateTime();
    }
    return record;
}

QVector<FileHistory> FileHistory::getAllRecords(QSqlDatabase& db, int limit) {
    QSqlQuery query(db);
    query.prepare("SELECT * FROM file_history ORDER BY created_at DESC LIMIT ?");
    query.addBindValue(limit);
    
    QVector<FileHistory> records;
    if (query.exec()) {
        while (query.next()) {
            FileHistory record;
            record.id = query.value("id").toInt();
            record.fileName = query.value("file_name").toString();
            record.filePath = query.value("file_path").toString();
            record.fileType = query.value("file_type").toString();
            record.fileSize = query.value("file_size").toLongLong();
            record.modifiedDate = query.value("modified_date").toDateTime();
            record.createdAt = query.value("created_at").toDateTime();
            records.append(record);
        }
    }
    return records;
}

FileHistoryManager::FileHistoryManager(const QString& dbPath) {
    db_ = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE"));
    db_->setDatabaseName(dbPath.isEmpty() ? getDatabasePath() : dbPath);
    
    if (!initDatabase()) {
        spdlog::error("Failed to initialize database");
    }
}

FileHistoryManager::~FileHistoryManager() {
    if (db_ && db_->isOpen()) {
        db_->close();
    }
}

bool FileHistoryManager::initDatabase() {
    if (!db_->open()) {
        spdlog::error("Failed to open database: {}", db_->lastError().text().toStdString());
        return false;
    }
    
    FileHistory::initTable(*db_);
    return true;
}

QString FileHistoryManager::getDatabasePath() const {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.filePath("file_history.db");
}

bool FileHistoryManager::addFileHistory(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return false;
    }

    FileHistory record;
    record.fileName = fileInfo.fileName();
    record.filePath = fileInfo.absoluteFilePath();
    record.fileType = fileInfo.suffix();
    record.fileSize = fileInfo.size();
    record.modifiedDate = fileInfo.lastModified();
    
    return FileHistory::addRecord(*db_, record);
}

bool FileHistoryManager::updateFileHistory(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return false;
    }

    FileHistory record;
    record.fileName = fileInfo.fileName();
    record.filePath = fileInfo.absoluteFilePath();
    record.fileType = fileInfo.suffix();
    record.fileSize = fileInfo.size();
    record.modifiedDate = fileInfo.lastModified();
    
    return FileHistory::updateRecord(*db_, record);
}

bool FileHistoryManager::deleteFileHistory(const QString& filePath) {
    return FileHistory::deleteRecord(*db_, filePath);
}

FileHistory FileHistoryManager::getFileHistory(const QString& filePath) {
    return FileHistory::getRecord(*db_, filePath);
}

QVector<FileHistory> FileHistoryManager::getRecentFiles(int limit) {
    return FileHistory::getAllRecords(*db_, limit);
}