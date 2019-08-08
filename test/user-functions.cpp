#include "SQLiteWrapper.h"
#include <sstream>
#include <iomanip>

static const char db_name[] = ":memory:";
using db = sqlite::Database<db_name>;

int main(void) {
  static const char increment_name[] = "increment";
  sqlite::createFunction<increment_name>([] (int x) {
    return x+1;
  });

  static const char quote_name[] = "quote";
  sqlite::createFunction<quote_name>([] (std::string_view str) {
    std::ostringstream ss;
    ss << std::quoted(str);
    return ss.str();
  });

  static const char create_table_query[] = "create table test (a, b)";
  db::query<create_table_query>();

  static const char insert_query[]
      = "insert into test (a, b) values (increment(increment(?1)), ?2)";
  db::query<insert_query>(1, "hello world");

  static const char select_query[]
    = "select increment(increment(a)), quote(quote(b)) from test where a = ?1";
  auto fetch_row = db::query<select_query>(3);
  int x;
  std::string str;
  while (fetch_row(x, str)) {
    assert(x == 5);
    assert(str == R"("\"hello world\"")");
  }
}
