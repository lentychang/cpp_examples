#include <sqlite3.h>

#include <iostream>

#include "db_static_interface.h"

// ConnectionString::ConnectionString(std::string a="", std::string b="", std::string c="", std::string d="") {}

//SQLite3 result code https://www.sqlite.org/rescode.html
class SqliteResult : public ResultBase<SqliteResult> {
    friend ResultBase<SqliteResult>;
    sqlite3_stmt* stmt;
    bool has_next;

protected:
public:
    explicit SqliteResult(sqlite3_stmt* stmt_) : stmt{stmt_}, has_next{false} {}
    ~SqliteResult() {
        if (stmt != nullptr)
            sqlite3_finalize(stmt);
    }
};

template <>
bool ResultBase<SqliteResult>::next() {
    if (ResultImpl->stmt != nullptr) {
        int res = sqlite3_step(ResultImpl->stmt);

        if (res == SQLITE_ROW) {
            ResultImpl->has_next = true;
            return true;
        } else if (res == SQLITE_DONE)
            return false;
        else
            std::cerr << "SQLITE Error Code: (" << res << ")\n";
    } else
        std::cerr << "SqliteResult.stmt is empty!!\n";
    return false;
}
template <>
void ResultBase<SqliteResult>::set_colname_idx_map() {
    for (int i = 0; i < sqlite3_column_count(ResultImpl->stmt); ++i) {
        colname_idx_map[std::string(sqlite3_column_name(ResultImpl->stmt, i))] = i;
    }
}

template <>
void ResultBase<SqliteResult>::reset() {
    sqlite3_reset(ResultImpl->stmt);
}

template <>
std::unique_ptr<Row> ResultBase<SqliteResult>::get_current_row() {
    if (ResultImpl->stmt == nullptr) return nullptr;
    auto row_ptr = std::make_unique<Row>();
    for (int i = 0; i < sqlite3_column_count(ResultImpl->stmt); ++i) {
        row_ptr->push_back(reinterpret_cast<char const*>(sqlite3_column_text(ResultImpl->stmt, i)));
    }
    return std::move(row_ptr);
}

template <>
std::unique_ptr<Row> ResultBase<SqliteResult>::get_first_row() {
    if (ResultImpl->stmt != nullptr) {
        reset();
        if (next()) return std::move(get_current_row());
    }
    return nullptr;
}

template <>
std::string ResultBase<SqliteResult>::operator[](const std::string& colname) {
    auto row_ptr = get_current_row();
    return (*row_ptr)[colname_idx_map[colname]];
}
template <>
Table ResultBase<SqliteResult>::get_all_rows() {}

// ConnectorBaseImpl
class SqliteConnector : public DBConnectorBase<SqliteConnector>,
                        public Query<SqliteResult, sqlite3>,
                        public TransactionSyntax<SqliteConnector> {
    friend DBConnectorBase<SqliteConnector>;

protected:
    inline static sqlite3* db = nullptr;

public:
    SqliteConnector() {
        ConnectorImpl = this;
    }
    ~SqliteConnector() {
        if (db != nullptr) sqlite3_free(db);
    }
};

// Specialized template, if  DBConnectorImpl is SqliteConnect, then set exec template ResultImpl to ResultSqlite,
template <>
SqliteResult* Query<SqliteResult, sqlite3>::exec(const std::string& sql) {
    char* zErrMsg = 0;
    int res = sqlite3_exec(session, sql.c_str(), NULL, NULL, &zErrMsg);
    switch (res) {
        case SQLITE_OK:
            break;
        case SQLITE_ERROR:
            std::cerr << "Sqlite3: Execute SQL failed!\n";
            break;
    }
};

// Specialized template can not have default parameter
template <>
bool DBConnectorBase<SqliteConnector>::connect(const ConnectionString& conn_str_) {
    // since we cannot provide default parameter, we have set default here.
    std::string addr{};
    (conn_str_.addr.empty()) ? addr = ":memory:" : conn_str_.addr;
    conn_str.addr = addr;
    int rc;
    rc = sqlite3_open(addr.c_str(), &(SqliteConnector::db));
    if (rc == SQLITE_OK) connected = true;
    return rc;
}

template <>
bool DBConnectorBase<SqliteConnector>::reconnect() {
    return connect(conn_str);
}

template <>
SqliteResult* Query<SqliteResult, sqlite3>::create(const std::string& sql){};
template <>
SqliteResult* Query<SqliteResult, sqlite3>::update(const std::string& sql){};
template <>
SqliteResult* Query<SqliteResult, sqlite3>::del(const std::string& sql){};
template <>
SqliteResult* Query<SqliteResult, sqlite3>::select(const std::string& sql){};

template <>
const std::string TransactionSyntax<SqliteConnector>::commit_sql = "END";
template <>
const std::string TransactionSyntax<SqliteConnector>::rollback_sql = "ROLLBACK";
template <>
const std::string TransactionSyntax<SqliteConnector>::start_transaction_sql = "BEGIN";

template <>
void DBConnectorBase<SqliteConnector>::rollback() {
    // derived().exec(SqliteConnector::rollback_sql);
    ConnectorImpl->exec(SqliteConnector::rollback_sql);
}

template <>
void DBConnectorBase<SqliteConnector>::commit() {
    ConnectorImpl->exec(SqliteConnector::commit_sql);
}
template <>
void DBConnectorBase<SqliteConnector>::start_transaction() {
    ConnectorImpl->exec(SqliteConnector::start_transaction_sql);
}
