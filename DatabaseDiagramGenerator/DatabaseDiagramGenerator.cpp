#include "DatabaseDiagramGenerator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sqlite3.h>

DatabaseDiagramGenerator::DatabaseDiagramGenerator(const std::string& dbFile) : dbFilename(dbFile) {}

std::vector<std::string> DatabaseDiagramGenerator::getUserTables(const std::string& schemaName) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbFilename.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Невозможно открыть базу данных: " + dbFilename);
    }
    std::vector<std::string> tableNames;
    std::string query = "SELECT name FROM " + schemaName + ".sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%';";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        throw std::runtime_error("Не удалось подготовить SQL-запрос для получения таблиц.");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* tableName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (tableName) {
            tableNames.push_back(std::string(tableName));
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return tableNames;
}

std::vector<TableField> DatabaseDiagramGenerator::getTableFields(const std::string& tableName) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbFilename.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Невозможно открыть базу данных: " + dbFilename);
    }
    std::vector<TableField> fields;
    std::string query = "PRAGMA table_info(" + tableName + ");";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        throw std::runtime_error("Не удалось подготовить запрос для получения полей таблицы " + tableName);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TableField field;
        const char* fieldName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* fieldType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int pk = sqlite3_column_int(stmt, 5);
        if (fieldName && fieldType) {
            field.name = fieldName;
            field.type = fieldType;
            field.isPrimary = (pk > 0);
            fields.push_back(field);
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return fields;
}

std::vector<ForeignKey> DatabaseDiagramGenerator::getTableForeignKeys(const std::string& schemaName, const std::string& tableName) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbFilename.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Невозможно открыть базу данных: " + dbFilename);
    }
    std::vector<ForeignKey> fkeys;
    std::string query = "PRAGMA " + schemaName + ".foreign_key_list(" + tableName + ");";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        throw std::runtime_error("Не удалось подготовить запрос для внешних ключей таблицы " + tableName);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* refTable = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* fromCol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* toCol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        if (refTable && fromCol && toCol) {
            ForeignKey fk;
            fk.fromTable = tableName;
            fk.fromColumn = fromCol;
            fk.toTable = refTable;
            fk.toColumn = toCol;
            fkeys.push_back(fk);
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return fkeys;
}

std::string DatabaseDiagramGenerator::generateDiagram(const std::string& schemaName) {
    auto tables = getUserTables(schemaName);
    std::ostringstream oss;
    oss << "# Database diagram\n\n";
    for (const auto& table : tables) {
        oss << "table \"" << table << "\" {\n";
        auto fields = getTableFields(table);
        for (const auto& field : fields) {
            oss << "  " << field.name << ": " << field.type;
            if (field.isPrimary)
                oss << " (PK)";
            oss << "\n";
        }
        oss << "}\n\n";
    }
    for (const auto& table : tables) {
        auto fkeys = getTableForeignKeys(schemaName, table);
        for (const auto& fk : fkeys) {
            oss << "arrow " << fk.fromTable << "." << fk.fromColumn
                << " -> " << fk.toTable << "." << fk.toColumn << "\n";
        }
    }
    return oss.str();
}

std::string DatabaseDiagramGenerator::loadTemplate(const std::string& templatePath) {
    std::ifstream inFile(templatePath);
    if (!inFile.is_open())
        throw std::runtime_error("Не удалось открыть HTML-шаблон: " + templatePath);
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    return buffer.str();
}

void DatabaseDiagramGenerator::generateHtmlDiagramFile(const std::string& schemaName, const std::string& templatePath, const std::string& outputFilename) {
    std::string diagramText = generateDiagram(schemaName);
    std::string htmlTemplate = loadTemplate(templatePath);
    const std::string placeholder = "%%DIAGRAM_TEXT%%";
    size_t pos = htmlTemplate.find(placeholder);
    if (pos == std::string::npos)
        throw std::runtime_error("Маркер " + placeholder + " не найден в HTML-шаблоне.");
    htmlTemplate.replace(pos, placeholder.length(), diagramText);
    std::ofstream outFile(outputFilename);
    if (!outFile.is_open())
        throw std::runtime_error("Не удалось открыть файл для записи: " + outputFilename);
    outFile << htmlTemplate;
    outFile.close();
    std::cout << "HTML-файл с диаграммой успешно сгенерирован: " << outputFilename << std::endl;
}
