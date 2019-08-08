#include "SQLiteWrapper.h"

// This example illustrates how to create a table, populate it, and fetch rows
// from it, using the wrapper.

static const char db_name[] = "example.db";
using db = sqlite::Database<db_name>;

int foo (int a) {
  return a+1;
}

struct some_wrapper {
  std::string_view inner;
};

template<>
constexpr auto sqlite::user_deserialize_fn<some_wrapper> = [] (std::string_view x) {
  return some_wrapper{x};
};

std::string bar (some_wrapper x) {
  std::string y = std::string(x.inner);
  y.append(" HI");
  return y;
}

int main(void) {
  static const char foo_name[] = "foo";
  sqlite::createFunction<foo_name>([] (int a) { return a+1; });

  static const char bar_name[] = "bar";
  sqlite::createFunction<bar_name>(&bar);

  static const char create_table_query[]
    = R"(create table if not exists users (first_name text,
                                           last_name text,
                                           age integer,
                                           website text))";
  db::query<create_table_query>();

  static const char clear_table_query[] = R"(delete from users)";
  db::query<clear_table_query>();

  static const char insert_users_query[]
    = R"(insert into users values (?1, ?2, ?3, ?4))";
  db::query<insert_users_query>("John", "Doe", 29, "google.com");
  db::query<insert_users_query>("Mary", "Smith", 28, std::nullopt);
  db::query<insert_users_query>("James", "Smith", 20, "yahoo.com");

  static const char select_users_query[]
    = R"(select first_name, last_name, foo(age), bar(bar(website))
         from users where age = ?1 or substr(first_name, 1, 1) = ?2)";
  auto fetch_row = db::query<select_users_query>(29, "M");

  std::string_view first_name, last_name;
  int age;
  std::optional<std::string_view> website;
  while (fetch_row(first_name, last_name, age, website)) {
    std::cout << first_name << " "
              << last_name << ", "
              << age << ", "
              << (website ? *website : "<no website>")
              << std::endl;
  }
}
