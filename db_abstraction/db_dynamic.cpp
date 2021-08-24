#include <driver.h>
#include <exception.h>
#include <mysql_connection.h>
#include <prepared_statement.h>
#include <resultset.h>
#include <sqlite3.h>
#include <statement.h>

#include <cstring>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::vector;
using Row = std::vector<std::string>;

std::unique_ptr<std::string> istreamToBuffer(std::istream* istrm) {
    auto strm_buff = istrm->rdbuf();
    istrm->seekg(0, istrm->end);
    std::size_t blob_size = istrm->tellg();
    istrm->seekg(0, istrm->beg);

    // (1) using sgetn
    char* buffer = new char[blob_size];
    strm_buff->sgetn(buffer, blob_size);
    // (2) chunk read
    // https://www.cplusplus.com/reference/istream/istream/read/
    return std::make_unique<std::string>(buffer, blob_size);
}

// TODO Blob Struct to convert char* and istream
class Blob : public std::streambuf {
protected:
    size_t size;
    std::unique_ptr<char[]> buff;
    std::istream* istrm;

public:
    explicit Blob(size_t size_) : size{size_}, buff{new char[size]}, istrm{nullptr} {
        this->pubsetbuf(buff.get(), size);
        istrm->rdbuf()->pubsetbuf(buff.get(), size);
    }
    Blob(const char* buff_, size_t size_) : size{size_}, buff{new char[size]}, istrm{nullptr} {
        std::memcpy(buff.get(), buff_, size);
        this->pubsetbuf(buff.get(), size);
        istrm->rdbuf()->pubsetbuf(buff.get(), size);
    }
    Blob(std::unique_ptr<char> buff_, size_t size_) : size{size_}, buff{buff_.release()}, istrm{nullptr} {
        this->pubsetbuf(buff.get(), size);
        istrm->rdbuf()->pubsetbuf(buff.get(), size);
    }
    explicit Blob(std::istream* istrm_) : size{0}, buff{nullptr}, istrm{istrm_} {
        auto pstr = istreamToBuffer(istrm);
        size = pstr->size();
        buff = std::unique_ptr<char[]>(new char[size]);
        istrm->read(buff.get(), size);
        if (!*istrm) std::cerr << "Error: Failed to read all from stream. Only " << istrm->gcount() << " out of " << size << "is read.\n";
        istrm->seekg(0, istrm->beg);
    }

    virtual int overflow(int c) override {
        std::cerr << "overflow: " << c << " ";
        return c;
    }
    virtual ~Blob() {}
    char* begin() const { return this->pbase(); }
    char* end() const { return this->pptr(); }
    std::istream* getIstreamPtr() {
        return istrm;
    }
    std::span<char> getBuffer() {
        return {begin(), size};
    }
    size_t getSize() { return size; }
    // This is not garanteed function, istream might only read a small part of data into buffer
    // TODO batch copy rdbuf to ostream, and check istrm is read to end.
    void copyIstrmToOstrm(std::ostream& os) {
        os.rdbuf(istrm->rdbuf());
    }
};

class ResultSetBase {
protected:
    virtual void reset() = 0;
    std::unordered_map<std::string, int> colname_idx_map;
    virtual void set_column_name_map() = 0;
    ResultSetBase() : colname_idx_map{} {}

public:
    virtual ~ResultSetBase() {}
    virtual int next() = 0;

    virtual int getInt(size_t col_idx) = 0;
    virtual float getFloat(size_t col_idx) = 0;
    virtual double getDouble(size_t col_idx) = 0;
    virtual char const* viewString(size_t col_idx) = 0;
    virtual std::string getString(size_t col_idx) = 0;
    virtual Blob* getBlob(size_t col_idx) = 0;
    virtual bool getBool(size_t col_idx) = 0;

    virtual std::string operator[](const std::string& colname) = 0;
    virtual std::string at(const std::string& colname) { return (*this)[colname]; };
    virtual std::unique_ptr<Row> get_current_row() = 0;
    virtual std::unique_ptr<std::vector<Row>> get_all_rows() = 0;
};

//SQLite3 result code https://www.sqlite.org/rescode.html
class SqliteResult : public ResultSetBase {
    struct Deleter {
        void operator()(sqlite3_stmt* stmt) {
            if (stmt) sqlite3_finalize(stmt);
        }
    };
    std::unique_ptr<sqlite3_stmt, Deleter> stmt;

protected:
    void set_column_name_map() override {
        for (int i = 0; i < sqlite3_column_count(stmt.get()); ++i) {
            colname_idx_map[sqlite3_column_name(stmt.get(), i)] = i;
        }
    }
    explicit SqliteResult(sqlite3_stmt* stmt_) : ResultSetBase{}, stmt{stmt_} {
    }

public:
    struct ResultFactory {
        static SqliteResult* create(sqlite3_stmt* stmt_) {
            auto result = new SqliteResult(stmt_);
            result->set_column_name_map();
            return result;
        }
    };
    virtual ~SqliteResult() {
    }
    int getInt(size_t col_idx) override {
        return sqlite3_column_int(stmt.get(), col_idx);
    }
    float getFloat(size_t col_idx) override {
        return sqlite3_column_double(stmt.get(), col_idx);
    }
    double getDouble(size_t col_idx) override {
        return sqlite3_column_double(stmt.get(), col_idx);
    }
    char const* viewString(size_t col_idx) override {
        return reinterpret_cast<char const*>(sqlite3_column_text(stmt.get(), col_idx));
    }
    std::string getString(size_t col_idx) override {
        return std::string(reinterpret_cast<char const*>(sqlite3_column_text(stmt.get(), col_idx)));
    }

    // TODO Sqlite getBlob
    // Should return unique_ptr, otherwise might have memory leak
    // This will copy the whole blob!!
    // For large data it will crash!
    // Use blob read instead https://www.sqlite.org/c3ref/blob_read.html
    Blob* getBlob(size_t col_idx) override {
        size_t blob_bytes = sqlite3_column_bytes(stmt.get(), col_idx);
        const void* blob = sqlite3_column_blob(stmt.get(), col_idx);
        return new Blob(reinterpret_cast<const char*>(blob), blob_bytes);
    }
    bool getBool(size_t col_idx) override { return getInt(col_idx); }

    int next() override {
        int res = sqlite3_step(stmt.get());
        if (res != SQLITE_ROW && res != SQLITE_DONE)
            std::cerr << "SQLITE Error Code: (" << res << ")\n";
        return res;
    }

    std::string operator[](const std::string& colname) override {
        auto row = (*get_current_row());
        return row[colname_idx_map[colname]];
    }

    void reset() override {
        sqlite3_reset(stmt.get());
    }

    std::unique_ptr<Row> get_current_row() override {
        if (stmt == nullptr) return nullptr;
        auto row_ptr = std::make_unique<Row>();
        for (int i = 0; i < sqlite3_column_count(stmt.get()); ++i) {
            row_ptr->push_back(getString(i));
        }
        return row_ptr;
    }
    std::unique_ptr<std::vector<Row>> get_all_rows() override {
        std::unique_ptr<std::vector<Row>> table{};
        reset();
        while (next() == SQLITE_ROW) {
            table->emplace_back(*get_current_row());
        }
        return table;
    }
};

class MysqlResult : public ResultSetBase {
    std::unique_ptr<sql::ResultSet> resultSet;

protected:
    void
    set_column_name_map() override {
        auto& meta = *resultSet->getMetaData();
        unsigned int nc = meta.getColumnCount();
        for (unsigned int i = 0; i < nc; ++i) {
            colname_idx_map[meta.getColumnName(i)] = i;
        }
    }
    void reset() override {
        // resultSet.reset(resultSet->getStatement()->getResultSet());
    }

public:
    static MysqlResult* CreateResult(sql::ResultSet* resultSet_) {
        auto result = new MysqlResult(resultSet_);
        result->set_column_name_map();

        return result;
    }
    explicit MysqlResult(sql::ResultSet* result_) : ResultSetBase{}, resultSet{result_} {}
    int getInt(size_t col_idx) override { return resultSet->getInt(col_idx); }
    float getFloat(size_t col_idx) override { return resultSet->getDouble(col_idx); }
    double getDouble(size_t col_idx) override { return resultSet->getDouble(col_idx); }
    char const* viewString(size_t col_idx) override { return resultSet->getString(col_idx).asStdString(); }
    // std::string getString(size_t col_idx) override {
    //     return std::string(resultSet->getString(col_idx).asStdString());
    // }
    Blob* getBlob(size_t col_idx) override {
        auto istrm = resultSet->getBlob(col_idx);
        return new Blob(istrm);  // TEST Mysql getBlob: sgetn or chunk read
    }

    bool getBool(size_t col_idx) override { return resultSet->getBoolean(col_idx); }

    std::string operator[](const std::string& colname) override {
        // auto idx = resultSet->findColumn(colname);
        // return resultSet->getString(idx);
        return resultSet->getString(colname_idx_map[colname]).asStdString();
    };
    std::string at(const std::string& colname) override { return (*this)[colname]; };

    int next() override { return resultSet->next(); };
    std::unique_ptr<Row> get_current_row() override {
        auto rowPtr = std::make_unique<Row>();
        for (size_t i = 0; i < colname_idx_map.size(); ++i) {
            rowPtr->push_back(getString(i));
        }
        return rowPtr;
    }
    std::unique_ptr<std::vector<Row>> get_all_rows() override {
        std::unique_ptr<std::vector<Row>> table{};
        reset();
        while (next() == 1) {
            table->emplace_back(*get_current_row());
        }
        return table;
    }
};

struct ConnectionParam {
    std::string addr;
    std::string port;
    std::string username;
    std::string pw;
};

struct SqlSyntaxBase {
    const std::string start_transaction;
    const std::string rollback;
    const std::string commit;
    SqlSyntaxBase(std::string const& start_transaction_, std::string const& rollback_, std::string const& commit_) : start_transaction{start_transaction_}, rollback{rollback_}, commit{commit_} {}
    virtual ~SqlSyntaxBase(){};
};

struct SqliteSyntax : public SqlSyntaxBase {
    SqliteSyntax() : SqlSyntaxBase{"BEGIN TRANSACTION", "ROLLBACK", "END"} {}
    virtual ~SqliteSyntax() {}
};

struct MysqlSyntax : public SqlSyntaxBase {
    MysqlSyntax() : SqlSyntaxBase{"START TRANSACTION", "ROLLBACK", "COMMIT"} {}
    virtual ~MysqlSyntax() {}
};

class StatementBase {
public:
    virtual void setInt(size_t idx, int val) = 0;
    virtual void setString(size_t idx, const std::string& str) = 0;
    virtual void setBlob(size_t idx, Blob* blob) = 0;
    virtual void setFloat(size_t idx, float val) = 0;
    virtual void setDouble(size_t idx, double val) = 0;
    virtual bool execute() = 0;
};

class SqliteStatement : public StatementBase {
    struct Deleter {
        void operator()(sqlite3_stmt* stmt) {
            if (stmt) sqlite3_finalize(stmt);
        }
    };
    std::unique_ptr<sqlite3_stmt, Deleter> stmt;

public:
    SqliteStatement(sqlite3_stmt* stmt_) : stmt{stmt_} {}
    virtual ~SqliteStatement() {}
    void setInt(size_t idx, int val) override { sqlite3_bind_int(stmt.get(), idx, val); }
    // fourth argument is the byte offset(size fo string) where the null terminator is assume to occur.
    // if -1 then it the string will be cut if null occur in the middle of string
    // The string's life time should be managed by user and should not be destruct before finalizaed.
    void setString(size_t idx, const std::string& str) override { sqlite3_bind_text(stmt.get(), idx, str.c_str(), str.size(), SQLITE_STATIC); }

    // TODO Sqlite3 setBlob
    // check uint8_t and char and unsigned char for blob
    // The blob's life time should be managed by user and should not be destruct before finalizaed.
    void setBlob(size_t idx, Blob* blob) override {
        auto buffer = blob->getBuffer();
        sqlite3_bind_blob(stmt.get(), idx, buffer.data(), buffer.size(), SQLITE_STATIC);
    }
    void setFloat(size_t idx, float val) override { sqlite3_bind_double(stmt.get(), idx, val); }
    void setDouble(size_t idx, double val) override { sqlite3_bind_double(stmt.get(), idx, val); }
    bool execute() override {
        int res = sqlite3_step(stmt.get());
        if (res == SQLITE_OK) return true;
        std::cerr << "SQLITE Error Code: (" << res << ")\n";
        return false;
    }
};

class MysqlPrepareStatement : public StatementBase {
    std::unique_ptr<sql::PreparedStatement> stmt;

public:
    explicit MysqlPrepareStatement(sql::PreparedStatement* stmt_) : stmt{stmt_} {}
    virtual ~MysqlPrepareStatement() {}
    void setInt(size_t idx, int val) override { stmt->setInt(idx, val); }
    void setString(size_t idx, const std::string& str) override { stmt->setString(idx, str); }
    // TODO Mysql setBlob
    // Blob const* blob cannot be bind to reference &blob->blob
    // The blob's life time should be managed by user and should not be destruct before finalizaed.
    void setBlob(size_t idx, Blob* blob) override {
        stmt->setBlob(idx, blob->getIstreamPtr());
    }
    void setFloat(size_t idx, float val) override { stmt->setDouble(idx, val); }
    void setDouble(size_t idx, double val) override { stmt->setDouble(idx, val); }
    bool execute() override {
        try {
            stmt->execute();
        } catch (...) {
            std::cerr << "Failed to execute PrepareStatement\n";
            return false;
        }
        return true;
    }
};

class DBConnectorBase {
protected:
    bool connected;
    inline static bool transaction_finished = true;
    std::mutex mtx;
    ConnectionParam conn_param;
    SqlSyntaxBase* sql_syntax;

public:
    DBConnectorBase(SqlSyntaxBase* sql_syntax_ = nullptr) : connected{false}, mtx{}, conn_param{"", "", "", ""}, sql_syntax{sql_syntax_} {}
    bool isConnected() { return connected; }

    virtual ResultSetBase* executeQuery(std::string const& sql) = 0;
    virtual void execute(std::string const& sql) = 0;
    virtual StatementBase* prepareStatement(std::string const& sql) = 0;

    virtual bool connect(const ConnectionParam& conn_param_) = 0;
    virtual bool reconnect() {
        while (!connected) { connect(conn_param); }
        return true;
    }
    void start_transaction() {
        if (transaction_finished) {
            std::scoped_lock lck{mtx};
            execute(sql_syntax->start_transaction);
            transaction_finished = false;
        }
    }
    void rollback() {
        if (!transaction_finished) {
            std::scoped_lock lck{mtx};
            std::cout << "\nRollback\n";
            execute(sql_syntax->rollback);
            transaction_finished = true;
        }
    }
    void commit() {
        if (!transaction_finished) {
            std::scoped_lock lck{mtx};
            execute(sql_syntax->commit);
            transaction_finished = true;
        }
    }
    virtual ~DBConnectorBase(){};
};

class SqliteConnector : public DBConnectorBase {
    using super = DBConnectorBase;
    SqliteConnector() : DBConnectorBase(new SqliteSyntax), db{nullptr} {
    }

protected:
    sqlite3* db;

public:
    static SqliteConnector* create_connector(std::string const& db_filepath = ":memory:") {
        auto conn = new SqliteConnector;
        conn->conn_param.addr = db_filepath;
        conn->connected = true;
        conn->connect({db_filepath});
        return conn;
    }
    ~SqliteConnector() {
        sqlite3_free(db);
    }

    virtual bool connect(const ConnectionParam& conn_param_ = ConnectionParam{":memory:", "", "", ""}) override {
        conn_param = conn_param_;
        int rc;
        rc = sqlite3_open(conn_param.addr.c_str(), &db);
        if (rc != SQLITE_OK) std::cerr << "Failed to oppen Sqlite db: " << conn_param.addr << '\n';
        return rc;
    }

    virtual SqliteResult* executeQuery(std::string const& sql) override {
        sqlite3_stmt* stmt;
        int res = sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);

        if (res != SQLITE_OK) {
            std::cerr << "Sqlite3: prepare SQL failed! error code(" << res << ")\n";
            std::cerr << "Refer to the following link for result code from sqlite3:\n"
                      << "https://www.sqlite.org/rescode.html\n";
            return nullptr;
        }
        return SqliteResult::ResultFactory::create(stmt);
    };
    virtual void execute(std::string const& sql) override {
        char* errMsg = nullptr;

        sqlite3_exec(db, sql.c_str(), NULL, NULL, &errMsg);
        if (errMsg != nullptr) {
            std::cerr << "Error calling excute: " << errMsg << '\n';
            sqlite3_free(errMsg);
            rollback();
        }
    }
    virtual SqliteStatement* prepareStatement(std::string const& sql) override {
        sqlite3_stmt* stmt;
        int res = sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
        switch (res) {
            case SQLITE_OK:
                return new SqliteStatement(stmt);
            default: {
                std::cerr << "Sqlite3: prepare SQL failed! error code(" << res << ")\n";
                std::cerr << "Refer to the following link for result code from sqlite3:\n"
                          << "https://www.sqlite.org/rescode.html\n";
            }
        }
        return nullptr;
    };
};

class MysqlConnector : public DBConnectorBase {
    sql::Driver* driver;
    sql::Connection* db;
    sql::Statement* stmt;

protected:
    MysqlConnector() : driver{get_driver_instance()}, db{nullptr} {}

public:
    static MysqlConnector* create_connector(std::string const& addr = "127.0.0.1", std::string const& port = "3306", std::string const& user = "mysql", std::string const& pw = "") {
        MysqlConnector* conn = new MysqlConnector();
        conn->conn_param.addr = addr;
        conn->conn_param.port = port;
        conn->conn_param.username = user;
        conn->conn_param.pw = pw;
        if (conn->connect(conn->conn_param)) conn->connected = true;
        return conn;
    }
    bool connect(const ConnectionParam& conn_param_) override {
        conn_param = conn_param_;
        db = driver->connect(conn_param.addr + ":" + conn_param.port, conn_param.username, conn_param.pw);
        (db) ? connected = true : false;
        return connected;
    }
    ~MysqlConnector() override {}
    MysqlResult* executeQuery(std::string const& sql) override {
        return new MysqlResult(stmt->executeQuery(sql));
    }
    void execute(std::string const& sql) override {
        db->createStatement()->execute(sql);
    }
    MysqlPrepareStatement* prepareStatement(std::string const& sql) override {
        return new MysqlPrepareStatement(db->prepareStatement(sql));
    }
};

int main() {
    auto context = SqliteConnector::create_connector();
    context->connect({"/home/lenty/test.db"});

    context->rollback();
    // conn->exec("create table test (a int, b varchar(20))");

    return 0;
}