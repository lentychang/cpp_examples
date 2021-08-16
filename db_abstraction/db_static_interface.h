#ifndef __DB_STATIC_INTERFACE_H__
#define __DB_STATIC_INTERFACE_H__

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

struct ConnectionString {
    std::string addr;
    std::string port;
    std::string username;
    std::string pw;
};

template <class ResultImpl, class SessionType>
class Query {
    SessionType* session;

public:
    ResultImpl* exec(const std::string& sql);
    ResultImpl* create(const std::string& sql);
    ResultImpl* update(const std::string& sql);
    ResultImpl* del(const std::string& sql);
    ResultImpl* select(const std::string& sql);
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
    bool connect(const ConnectionString& conn_str);
    bool reconnect();

    void rollback();
    void start_transaction();
    void commit();

    static bool isConnected() { return connected; }
};

#endif  // __DB_STATIC_INTERFACE_H__