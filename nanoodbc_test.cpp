#include <iostream>
#include <fstream>
#include <nanodbc/nanodbc.h>
#include <string>
#include <cstddef>

using namespace std;
using namespace nanodbc;
void show(nanodbc::result &results);

#ifdef NANODBC_ENABLE_UNICODE
inline nanodbc::string convert(std::string const &in)
{
  static_assert(
      sizeof(nanodbc::string::value_type) > 1,
      "NANODBC_ENABLE_UNICODE mode requires wide string");
  nanodbc::string out;
#if defined(__GNUC__) && __GNUC__ < 5
  std::vector<wchar_t> characters(in.length());
  for (size_t i = 0; i < in.length(); i++)
    characters[i] = in[i];
  const wchar_t *source = characters.data();
  size_t size = wcsnrtombs(nullptr, &source, characters.size(), 0, nullptr);
  if (size == std::string::npos)
    throw std::range_error("UTF-16 -> UTF-8 conversion error");
  out.resize(size);
  wcsnrtombs(&out[0], &source, characters.size(), out.length(), nullptr);
#elif defined(_MSC_VER) && (_MSC_VER >= 1900)
  // Workaround for confirmed bug in VS2015 and VS2017 too
  // See: https://connect.microsoft.com/VisualStudio/Feedback/Details/1403302
  using wide_char_t = nanodbc::string::value_type;
  auto s =
      std::wstring_convert<std::codecvt_utf8_utf16<wide_char_t>, wide_char_t>().from_bytes(in);
  auto p = reinterpret_cast<wide_char_t const *>(s.data());
  out.assign(p, p + s.size());
#else
  out = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().from_bytes(in);
#endif
  return out;
}

inline std::string convert(nanodbc::string const &in)
{
  static_assert(sizeof(nanodbc::string::value_type) > 1, "string must be wide");
  std::string out;
#if defined(__GNUC__) && __GNUC__ < 5
  size_t size = mbsnrtowcs(nullptr, in.data(), in.length(), 0, nullptr);
  if (size == std::string::npos)
    throw std::range_error("UTF-8 -> UTF-16 conversion error");
  std::vector<wchar_t> characters(size);
  const char *source = in.data();
  mbsnrtowcs(&characters[0], &source, in.length(), characters.size(), nullptr);
  out.resize(size);
  for (size_t i = 0; i < in.length(); i++)
    out[i] = characters[i];
#elif defined(_MSC_VER) && (_MSC_VER >= 1900)
  // Workaround for confirmed bug in VS2015 and VS2017 too
  // See: https://connect.microsoft.com/VisualStudio/Feedback/Details/1403302
  using wide_char_t = nanodbc::string::value_type;
  std::wstring_convert<std::codecvt_utf8_utf16<wide_char_t>, wide_char_t> convert;
  auto p = reinterpret_cast<const wide_char_t *>(in.data());
  out = convert.to_bytes(p, p + in.size());
#else
  out = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().to_bytes(in);
#endif
  return out;
}
#else
inline nanodbc::string convert(std::string const &in)
{
  return in;
}
#endif

class Buffer
{
  char *m_buffer;
  size_t m_size;

public:
  Buffer() : m_buffer{nullptr}, m_size{0} {}
  Buffer(char *raw_buffer, size_t size) : m_buffer{raw_buffer}, m_size{size} {}
  inline char *get_buffer() const { return m_buffer; }
  inline size_t get_size() const { return m_size; }
  ~Buffer()
  {
    // if (m_buffer != nullptr)
    // {
    //   delete[] m_buffer;
    //   m_buffer = nullptr;
    // }
  }
};

Buffer
fileToBuffer(const std::string &fpath)
{
  std::ifstream file(fpath, std::ios::in | std::ios::binary);
  if (!file)
  {
    cerr << "An error occurred opening the file\n";
    return Buffer{};
  }
  file.seekg(0, std::ifstream::end);
  std::streampos size = file.tellg();
  file.seekg(0);

  char *buffer = new char[size];
  file.read(buffer, size);
  return Buffer{buffer, static_cast<std::size_t>(size)};
}

void bufferToFile(Buffer buff, const std::string &fpath)
{
  auto raw_buff = buff.get_buffer();
  size_t s = buff.get_size();
  std::ofstream of{fpath, ios_base::out | ios_base::binary};
  of.write(raw_buff, s);
}

int main()
{
  std::string connection_string = "Driver=SQLite3;Database=:memory:";
  nanodbc::connection connection(connection_string);
  nanodbc::result results;
  execute(connection, "create table simple_test (a int, b varchar(10));");

  // Direct execution
  {
    std::cout << "Section [Direct Execution]";
    execute(connection, "insert into simple_test values (1, 'one');");
    execute(connection, "insert into simple_test values (2, 'two');");
    execute(connection, "insert into simple_test values (3, 'tri');");
    execute(connection, "insert into simple_test (b) values ('z');");
    results = execute(connection, "select * from simple_test;");
    show(results);
  }

  // Accessing results by name, or column number
  {
    std::cout << "\nSection [Accessing results by name, or column number]\n";
    results = execute(
        connection, "select a as first, b as second from simple_test where a = 1;");
    results.next();
    cout << endl
         << results.get<int>("first") << ", " << results.get<string>(1) << endl;
  }

  // Binding parameters
  {
    std::cout << "\nSection [Binding parameters]";
    nanodbc::statement statement(connection);

    // Inserting values
    prepare(statement, "insert into simple_test (a, b) values (?, ?);");
    const int eight_int = 8;
    statement.bind(0, &eight_int);
    const string eight_str = "eight";
    statement.bind(1, eight_str.c_str());
    execute(statement);

    // Inserting null values
    prepare(statement, "insert into simple_test (a, b) values (?, ?);");
    statement.bind_null(0);
    statement.bind_null(1);
    execute(statement);

    // Inserting multiple null values
    prepare(statement, "insert into simple_test (a, b) values (?, ?);");
    statement.bind_null(0, 2);
    statement.bind_null(1, 2);
    execute(statement, 2);

    prepare(statement, "select * from simple_test;");
    results = execute(statement);
    show(results);
  }

  // BLOB insertion
  // Binary bind is a bug in nanodbc
  // {
  //   std::cout << "\nBLOB insertion";
  //   execute(connection, "create table blob_test (a int, b varchar(10), c blob);");
  //   nanodbc::statement statement(connection);

  //   // Inserting values
  //   prepare(statement, "insert into blob_test (a, b, c) values (?, ?, ?);");
  //   const int eight_int = 8;
  //   statement.bind(0, &eight_int);
  //   const string eight_str = "eight";
  //   statement.bind(1, eight_str.c_str());

  //   const auto buff = fileToBuffer("/home/lenty/what.JPG");

  //   uint8_t *s = new uint8_t[buff.get_size()];
  //   // std::uint8_t* s
  //   std::vector<uint8_t> t{buff.get_buffer(), buff.get_buffer() + buff.get_size()};
  //   // statement.bind(2, t.data());
  //   //

  //   results = execute(statement);
  //   prepare(statement, "select c from blob_test;");
  //   results = execute(statement);
  //   results.next();
  //   // auto blob = results.get<std::string>(0);
  //   auto blob = results.get<std::vector<uint8_t>>(0);
  //   int k = blob.size();
  //   uint8_t *f = blob.data();
  //   // bufferToFile(buff, "/home/lenty/new_pic2.jpg");
  //   bufferToFile(Buffer{reinterpret_cast<char *>(f), blob.size()}, "/home/lenty/new_pic.jpg");
  //   show(results);
  // }

  // Transactions
  {
    std::cout << "\nSection [Transactions]";
    {
      cout << "\ndeleting all rows ... " << flush;
      nanodbc::transaction transaction(connection);
      execute(connection, "delete from simple_test;");
      // transaction will be rolled back if we don't call transaction.commit()
    }
    results = execute(connection, "select count(1) from simple_test;");
    results.next();
    cout << "still have " << results.get<int>(0) << " rows!" << endl;
  }

  // Batch inserting
  {
    std::cout << "\nSection [Batch inserting]";
    nanodbc::statement statement(connection);
    execute(connection, "drop table if exists batch_test;");
    execute(connection, "create table batch_test (x varchar(10), y int, z float);");
    prepare(statement, "insert into batch_test (x, y, z) values (?, ?, ?);");

    const std::size_t elements = 4;

    char xdata[elements][10] = {"this", "is", "a", "test"};
    statement.bind_strings(0, xdata);

    int ydata[elements] = {1, 2, 3, 4};
    statement.bind(1, ydata, elements);

    float zdata[elements] = {1.1, 2.2, 3.3, 4.4};
    statement.bind(2, zdata, elements);

    transact(statement, elements);

    results = execute(connection, "select * from batch_test;", 3);
    show(results);

    execute(connection, "drop table if exists batch_test;");
  }

  // Dates and Times
  {
    execute(connection, "drop table if exists date_test;");
    execute(connection, "create table date_test (x datetime);");
    execute(connection, "insert into date_test values (current_timestamp);");

    results = execute(connection, "select * from date_test;");
    results.next();

    nanodbc::date date = results.get<nanodbc::date>(0);
    cout << endl
         << date.year << "-" << date.month << "-" << date.day << endl;

    results = execute(connection, "select * from date_test;");
    show(results);

    execute(connection, "drop table if exists date_test;");
  }

  // Inserting NULL values with a sentry
  {
    nanodbc::statement statement(connection);
    prepare(statement, "insert into simple_test (a, b) values (?, ?);");

    const int elements = 5;
    const int a_null = 0;
    const char *b_null = "";
    int a_data[elements] = {0, 88, 0, 0, 0};
    char b_data[elements][10] = {"", "non-null", "", "", ""};

    statement.bind(0, a_data, elements, &a_null);
    statement.bind_strings(1, b_data, b_null);

    execute(statement, elements);

    nanodbc::result results = execute(connection, "select * from simple_test;");
    show(results);
  }

  // Inserting NULL values with flags
  {
    nanodbc::statement statement(connection);
    prepare(statement, "insert into simple_test (a, b) values (?, ?);");

    const int elements = 2;
    int a_data[elements] = {0, 42};
    char b_data[elements][10] = {"", "every"};
    bool nulls[elements] = {true, false};

    statement.bind(0, a_data, elements, nulls);
    statement.bind_strings(1, b_data, nulls);

    execute(statement, elements);

    nanodbc::result results = execute(connection, "select * from simple_test;");
    show(results);
  }

  // Cleanup
  execute(connection, "drop table if exists simple_test;");

  return 0;
}

void show(nanodbc::result &results)
{
  const short columns = results.columns();
  long rows_displayed = 0;

  cout << "\nDisplaying " << results.affected_rows() << " rows "
       << "(" << results.rowset_size() << " fetched at a time):" << endl;

  // show the column names
  cout << "idx\t";
  for (short i = 0; i < columns; ++i)
    cout << convert(results.column_name(i)) << "\t";
  cout << endl;

  // show the column data for each row
  nanodbc::string const null_value = NANODBC_TEXT("null");
  while (results.next())
  {
    cout << rows_displayed++ << "\t";
    for (short col = 0; col < columns; ++col)
    {
      auto const value = results.get<nanodbc::string>(col, null_value);
      cout << convert(value) << "\t";
    }
    cout << endl;
  }
}