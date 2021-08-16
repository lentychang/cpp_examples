#include <sqlite3.h>

#include <iomanip>
#include <iostream>
#include <vector>

int callback(void *unUsed, int argc, char **argv, char **azColName) {
    std::cout << "callback called, with argc: " << argc << " argv: " << **argv << std::endl;
    int i;
    for (i = 0; i < argc; i++) {
        std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << "\n";
    }
    std::cout << "\n";
    return 0;
};

int main() {
    sqlite3 *db;
    char *zErrMsg;
    int rc;
    rc = sqlite3_open(":memory:", &db);

    int (*FUNC_PTR)(void *, int, char **, char **);

    sqlite3_exec(db, "create table test (a int, b varchar(20))", callback, nullptr, nullptr);
    sqlite3_exec(db, "insert into test values(5,'testb')", callback, nullptr, nullptr);

    char *errmsg = 0;
    sqlite3_exec(db, "select * from test", callback, nullptr, &errmsg);

    sqlite3_stmt *stmt;  // :AAAA name_parameter

    int res = sqlite3_prepare_v2(db, "insert into test values(?1,?2)", -1, &stmt, nullptr);
    std::cout << "?1 index: " << sqlite3_bind_parameter_index(stmt, "?1") << std::endl;
    std::cout << "?2 index: " << sqlite3_bind_parameter_index(stmt, "?2") << std::endl;

    sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "?1"), 10);
    //stmt,pos,str,size of string, funcptr
    sqlite3_bind_text(stmt, 2, "testabc", 8, nullptr);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_free(errmsg);
    sqlite3_exec(db, "select * from test", callback, nullptr, &errmsg);

    res = sqlite3_prepare_v2(db, "insert into test values(:inta,:text)", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, ":inta"), 10);
    sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, ":text"), "abcde", 5, nullptr);
    //stmt,pos,str,size of string, funcptr
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_free(errmsg);
    sqlite3_exec(db, "select * from test", callback, nullptr, &errmsg);

    sqlite3_free(errmsg);

    sqlite3_stmt *stmt2;  // :AAAA name_parameter
    int res2 = sqlite3_prepare_v2(db, "select a,b from test", -1, &stmt2, nullptr);
    std::vector<sqlite3_value *> results;
    while (sqlite3_step(stmt2) == SQLITE_ROW) {
        int nc = sqlite3_column_count(stmt2);
        for (int i = 0; i < nc; ++i) {
            results.push_back(sqlite3_value_dup(sqlite3_column_value(stmt, i)));
        }
    }
    std::cout << std::setw(20) << std::setfill('=') << std::endl;
    std::cout << std::endl;
    for (size_t i = 0; i < results.size(); ++i) {
        sqlite3_value *a = results.at(i);
        std::cout << sqlite3_value_text(a) << " ";
        sqlite3_value_free(results[i]);
    }
    std::cout << std::endl;
    sqlite3_exec(db, "select * from test", callback, nullptr, &errmsg);

    sqlite3_close(db);
    return 0;
}