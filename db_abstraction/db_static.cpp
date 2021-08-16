#include <driver.h>
#include <exception.h>
#include <mysql_connection.h>
#include <resultset.h>
#include <sqlite3.h>
#include <statement.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

using Row = std::vector<std::string>;
using Table = std::vector<Row>;
template <class Impl>
class ResultBase {
    Impl* ResultImpl = nullptr;
    std::unordered_map<std::string, int> colname_idx_map;
    Row row;
    void set_colname_idx_map();

public:
    Impl& derived() { return *static_cast<Impl*>(this); }
    bool next();
    void reset();
    std::string operator[](const std::string& colname);
    std::unique_ptr<Row> get_current_row();
    std::unique_ptr<Row> get_first_row();
    Table get_all_rows();
};

template <class ResultImpl, class SessionType>
class Query {
    SessionType* session = nullptr;

public:
    ResultImpl* exec(const std::string& sql);
    ResultImpl* create(const std::string& sql);
    ResultImpl* update(const std::string& sql);
    ResultImpl* del(const std::string& sql);
    ResultImpl* select(const std::string& sql);
};

struct ConnectionString {
    std::string addr;
    std::string port;
    std::string username;
    std::string pw;
};

template <class ConnectorImpl>
struct TransactionSyntax {
    // this is same in sqlite and mysql
    static const std::string rollback_sql;
    // template <char const* TRANSACTION_START_SQL>
    static const std::string start_transaction_sql;
    static const std::string commit_sql;
};

template <class Impl>
class DBConnectorBase {
protected:
    inline static bool connected = true;
    inline static std::mutex mtx{};
    inline static Impl* ConnectorImpl = nullptr;
    inline static bool transaction_started = false;
    ConnectionString conn_str;

public:
    Impl& derived() { return *static_cast<Impl*>(this); }
    DBConnectorBase() : conn_str{"", "", "", ""} {}
    static Impl* GetInstance() {
        std::scoped_lock lck(mtx);
        if (ConnectorImpl == nullptr)
            ConnectorImpl = new Impl;
        return ConnectorImpl;
    };

    // Here default value is for convenience of sqlite
    bool connect(ConnectionString& conn_str);
    bool reconnect();

    void rollback();
    void start_transaction();
    void commit();

    static bool isConnected() { return connected; }
};

// ======================
// Sqlite3 Implementation
// ======================
//SQLite3 result code https://www.sqlite.org/rescode.html
class SqliteResult : public ResultBase<SqliteResult> {
    friend ResultBase<SqliteResult>;
    sqlite3_stmt* stmt = nullptr;
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
    SqliteConnector() {
        ConnectorImpl = this;
    }
    ~SqliteConnector() { sqlite3_free(db); }

public:
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
bool DBConnectorBase<SqliteConnector>::connect(ConnectionString& conn_str) {
    // since we cannot provide default parameter, we have set default here.
    if (conn_str.addr.empty()) conn_str.addr = ":memory:";
    int rc;
    rc = sqlite3_open(conn_str.addr.c_str(), &(SqliteConnector::db));
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

// template <>
// class MysqlConnector : public DBConnectorBase<MysqlConnector> {
// };

int main() {
    // auto conn = DBConnectorBase<SqliteConnector>::GetInstance();
    // conn->connect("test.db");
    sqlite3_stmt* stmt = nullptr;
    auto res = SqliteResult(stmt);
    // conn->exec("create table test (a int, b varchar(20))");
    return 0;
}