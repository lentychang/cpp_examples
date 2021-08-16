#include "db_static_sqlite.cpp"

int main() {
    // auto conn = DBConnectorBase<SqliteConnector>::GetInstance();
    // conn->connect("test.db");
    sqlite3_stmt* stmt = nullptr;
    auto res = SqliteResult(stmt);
    SqliteConnector session{};
    // ConnectionString connstr{""};
    session.connect({""});
    // conn->exec("create table test (a int, b varchar(20))");
    return 0;
}