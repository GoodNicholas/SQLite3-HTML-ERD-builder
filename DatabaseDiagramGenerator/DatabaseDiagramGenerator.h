#ifndef PARVPO5_DATABASEDIAGRAMGENERATOR_H
#define PARVPO5_DATABASEDIAGRAMGENERATOR_H

#include <string>
#include <vector>
#include <stdexcept>

struct TableField {
    std::string name;
    std::string type;
    bool isPrimary;
};

struct ForeignKey {
    std::string fromTable;
    std::string fromColumn;
    std::string toTable;
    std::string toColumn;
};

class DatabaseDiagramGenerator {
public:
    DatabaseDiagramGenerator(const std::string& dbFile);
    std::vector<std::string> getUserTables(const std::string& schemaName);
    std::vector<TableField> getTableFields(const std::string& tableName);
    std::vector<ForeignKey> getTableForeignKeys(const std::string& schemaName, const std::string& tableName);
    std::string generateDiagram(const std::string& schemaName);
    std::string loadTemplate(const std::string& templatePath);
    void generateHtmlDiagramFile(const std::string& schemaName,
                                 const std::string& templatePath,
                                 const std::string& outputFilename);
private:
    std::string dbFilename;
};

#endif //PARVPO5_DATABASEDIAGRAMGENERATOR_H
