#include "SQLiteWrapper.h"
#include <sstream>
#include <iomanip>

static const char db_name[] = ":memory:";
using db = sqlite::Database<db_name>;

struct noncopyable_string {
  noncopyable_string() = default;

  noncopyable_string &operator=(noncopyable_string &&) = default;
  noncopyable_string(noncopyable_string &&) = default;

  noncopyable_string(const noncopyable_string &) = delete;
  noncopyable_string &operator=(const noncopyable_string &) = delete;

  std::string str;
};

template<>
constexpr auto sqlite::user_serialize_fn<noncopyable_string> =
    [] (noncopyable_string str) -> std::string {
      return std::move(str.str);
    };

template<>
constexpr auto sqlite::user_deserialize_fn<noncopyable_string> =
    [] (std::string str) -> noncopyable_string {
      return noncopyable_string{std::move(str)};
    };

struct string_wrapper {
  std::string str;
};

template<>
constexpr auto sqlite::user_serialize_fn<string_wrapper> =
    [] (string_wrapper str) -> std::string {
      return std::move(str.str);
    };

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

  static const char quote_noncopy_name[] = "quote_noncopy";
  sqlite::createFunction<quote_noncopy_name>(
      [] (noncopyable_string str) -> noncopyable_string {
        std::ostringstream ss;
        ss << std::quoted(str.str);
        return noncopyable_string{ss.str()};
      });

  static string_wrapper dummy_string{"dummy string"};
  static const char dummy_string_name[] = "dummy_string";
  sqlite::createFunction<dummy_string_name>(
      [] (void) -> string_wrapper & {
        return dummy_string;
      });

  static const char create_table_query[] = "create table test (a, b)";
  db::query<create_table_query>();

  static const char insert_query[]
      = "insert into test (a, b) values (increment(increment(?1)), ?2)";
  db::query<insert_query>(1, "hello world");

  static const char select_query[]
    = R"(select increment(increment(a)), quote(quote(b)),
                quote_noncopy(quote_noncopy(b)) || quote(b)
         from test where a = ?1)";
  auto fetch_row = db::query<select_query>(3);
  int x;
  std::string str;
  noncopyable_string noncopy_str;
  while (fetch_row(x, str, noncopy_str)) {
    assert(x == 5);
    assert(str == R"("\"hello world\"")");
    assert(noncopy_str.get_str() == R"("\"hello world\"""hello world")");
  }

  static const char insert_noncopyable_query[] = "insert into test (a) values (?1)";
  db::query<insert_noncopyable_query>(noncopyable_string{"hello world"});

  static const char select_dummy_string_query[] = "select dummy_string()";
  fetch_row = db::query<select_dummy_string_query>();
  std::string str2;
  while (fetch_row(str2)) {
    ;
  }
  assert(dummy_string.str == "dummy string");

  sqlite::detail::maybe_invoke(noncopyable_string{"test"});
}
