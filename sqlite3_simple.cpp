#include <sqlite3.h>
#include <iostream>

int callback(void *unUsed, int argc, char **argv, char **azColName)
{
  std::cout << "callback called, with argc: " << argc << " argv: " << **argv << std::endl;
  int i;
  for (i = 0; i < argc; i++)
  {
    std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << "\n";
  }
  std::cout << "\n";
  return 0;
};

int main()
{
  sqlite3 *db;
  char *zErrMsg;
  int rc;
  rc = sqlite3_open(":memory:", &db);

  int (*FUNC_PTR)(void *, int, char **, char **);

  sqlite3_exec(db, "create table test (a int, b varchar(20))", callback, nullptr, nullptr);
  sqlite3_exec(db, "insert into test values(5,'testb')", callback, nullptr, nullptr);

  char **errmsg;
  sqlite3_exec(db, "select * from test", callback, nullptr, errmsg);

  sqlite3_stmt *stmt;
  int res = sqlite3_prepare_v2(db, "insert into test values(?,?)", -1, &stmt, nullptr);
  sqlite3_bind_int(stmt, 1, 10);
  //stmt,pos,str,size of string, funcptr
  sqlite3_bind_text(stmt, 2, "testabc", 8, nullptr);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  sqlite3_exec(db, "select * from test", callback, nullptr, errmsg);

  sqlite3_close(db);
  return 0;
}