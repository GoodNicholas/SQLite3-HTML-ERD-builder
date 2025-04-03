#include "DatabaseDiagramGenerator/DatabaseDiagramGenerator.h"
#include <iostream>
#include <cstdlib>

int main() {
    try {
        DatabaseDiagramGenerator generator("/Users/krotovnikolay/test.db"); //enter absolute path to the sqlite3 database
        auto tables = generator.getUserTables("main");
        std::cout << "Найденные таблицы в схеме 'main':\n";
        for (const auto& table : tables)
            std::cout << " - " << table << "\n";
        std::string templatePath = "prompt.html"; //enter absolute path of prompt file in this directory
        std::string outputHtml = ""; //enter absolute path to the result directory
        generator.generateHtmlDiagramFile("main", templatePath, outputHtml);
#ifdef _WIN32
        std::string command = "start " + outputHtml;
#elif __APPLE__
        std::string command = "open " + outputHtml;
#else
        std::string command = "xdg-open " + outputHtml;
#endif
        system(command.c_str());
    } catch (const std::exception& ex) {
        std::cerr << "Ошибка: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
