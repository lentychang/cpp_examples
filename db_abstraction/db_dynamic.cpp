#include <driver.h>
#include <exception.h>
#include <mysql_connection.h>
#include <resultset.h>
#include <sqlite3.h>
#include <statement.h>

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class ResultBase {
protected:
    std::unordered_map<std::string, int> colname_idx_map;
    std::vector<std::string> row;
    ResultBase* result_ptr;

public:
    virtual ResultBase* getInstance() { return result_ptr; }
    virtual bool next() = 0;
    virtual const std::string& operator[](const std::string& colname) {}
    virtual const std::vector<std::string>& get_current_row() = 0;
    virtual std::vector<std::vector<std::string>> get_all_rows() = 0;
};

//SQLite3 result code https://www.sqlite.org/rescode.html
class SqliteResult : public ResultBase {
protected:
    sqlite3_stmt* stmt;

public:
    SqliteResult() : stmt{nullptr} {}
    virtual bool next() override {
        if (stmt != nullptr) {
            int res = sqlite3_step(stmt);

            if (res == SQLITE_OK)
                return true;
            else if (res == SQLITE_DONE)
                return false;
            else
                std::cerr << "SQLITE Error Code: (" << res << ")\n";
            ;
        } else
            std::cerr << "SqliteResult.stmt is empty!!\n";
        return false;
    };
    virtual SqliteResult* getInstance() override { return dynamic_cast<SqliteResult*>(result_ptr); }
    virtual const std::string& operator[](const std::string& colname) override {}
    virtual const std::vector<std::string>& get_current_row() override {}
    virtual std::vector<std::vector<std::string>> get_all_rows() override {}
};

class MysqlResult : public ResultBase {
public:
    virtual MysqlResult* getInstance() { return dynamic_cast<MysqlResult*>(result_ptr); }
    virtual bool next() override{};
    virtual const std::string& operator[](const std::string& colname) override {}
    virtual const std::vector<std::string>& get_current_row() override{};
    virtual std::vector<std::vector<std::string>> get_all_rows() override{};
};

class DBConnectorBase {
protected:
    static bool connected;
    static std::mutex mtx;
    static DBConnectorBase* conn;
    std::string addr;
    std::string port;
    std::string username;
    std::string pw;

public:
    DBConnectorBase() : addr(""), port{""}, username{""}, pw{""} {}
    virtual DBConnectorBase* GetInstance() = 0;
    static bool isConnected() { return connected; }

    virtual ResultBase* exec(const std::string& sql) = 0;
    virtual ResultBase* create(const std::string& sql) = 0;
    virtual ResultBase* update(const std::string& sql) = 0;
    virtual ResultBase* del(const std::string& sql) = 0;
    virtual ResultBase* read(const std::string& sql) = 0;
    virtual bool connect(const std::string& addr_, const std::string& port_, const std::string& username_, const std::string& pw_) = 0;
    virtual bool reconnect() = 0;
    virtual ~DBConnectorBase(){};
};
std::mutex DBConnectorBase::mtx{};
DBConnectorBase* DBConnectorBase::conn = nullptr;

class SqliteConnector : public DBConnectorBase {
    using super = DBConnectorBase;
    SqliteConnector() : db{nullptr} {
        super::conn = this;
    }

protected:
    sqlite3* db;

public:
    static SqliteConnector* create() {
        std::scoped_lock lck(mtx);
        if (super::conn == nullptr)
            SqliteConnector();
        return dynamic_cast<SqliteConnector*>(super::conn);
    }
    ~SqliteConnector() { sqlite3_free(db); }
    virtual SqliteConnector* GetInstance() override {
        std::scoped_lock lck(mtx);
        if (super::conn == nullptr)
            super::conn = new SqliteConnector;
        return dynamic_cast<SqliteConnector*>(super::conn);
    }
    virtual bool connect(const std::string& filepath = ":memory:", const std::string& port_ = "", const std::string& username_ = "", const std::string& pw_ = "") override {
        super::addr = filepath;
        int rc;
        rc = sqlite3_open(addr.c_str(), &db);
        return rc;
    }
    virtual bool reconnect() override { connect(addr.c_str()); }

    virtual ResultBase* exec(const std::string& sql) override {
        char* zErrMsg = 0;
        int res = sqlite3_exec(SqliteConnector::GetInstance()->db, sql.c_str(), NULL, NULL, &zErrMsg);
        switch (res) {
            case SQLITE_OK:
                break;
            case SQLITE_ERROR:
                std::cerr << "Sqlite3: Execute SQL failed!\n";
                break;
        }
    }
    virtual SqliteResult* create(const std::string& sql) override{};
    virtual SqliteResult* update(const std::string& sql) override{};
    virtual SqliteResult* del(const std::string& sql) override{};
    virtual SqliteResult* read(const std::string& sql) override{};
};

class MysqlConnector : public DBConnectorBase {
public:
    virtual MysqlResult* exec(const std::string& sql) override{};
    virtual MysqlResult* create(const std::string& sql) override{};
    virtual MysqlResult* update(const std::string& sql) override{};
    virtual MysqlResult* del(const std::string& sql) override{};
    virtual MysqlResult* read(const std::string& sql) override{};
    virtual bool connect(const std::string& addr_, const std::string& port_, const std::string& username_, const std::string& pw_) override{};
    virtual bool reconnect() override{};
    virtual ~MysqlConnector() override{};
};

int main() {
    auto context = SqliteConnector::create();
    context->connect("test.db");
    // conn->exec("create table test (a int, b varchar(20))");

    return 0;
}