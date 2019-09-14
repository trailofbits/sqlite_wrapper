#include "SQLiteWrapper.h"
#include <cassert>

static const char db_name[] = ":memory:";
using db = sqlite::Database<db_name>;

static const char insert_query[] = "insert into test (a, b) values (?1, ?2)";
static const char select_query[] = "select a, b from test where a = ?1";
static const char clear_table_query[] = "delete from test";

void test_blob_and_blob_view(void) {
  db::query<insert_query>(1, sqlite::blob{"hello"});
  db::query<insert_query>(2, sqlite::blob_view{"goodbye"});

  db::QueryResult fetch_row = db::query<select_query>(1);
  sqlite::blob_view blob_view;
  while (fetch_row(std::nullopt, blob_view)) {
    assert(blob_view == "hello");
  }

  fetch_row = db::query<select_query>(2);
  sqlite::blob blob;
  while (fetch_row(std::nullopt, blob)) {
    assert(blob == "goodbye");
  }
}

void test_transactions(void) {
  try {
    db::TransactionGuard txn;

    db::query<insert_query>(1, sqlite::blob{"hello"});
    throw std::exception();
    db::query<insert_query>(2, sqlite::blob_view{"goodbye"});
  } catch (const std::exception &e) { }

  auto fetch_row = db::query<select_query>(1);
  assert(fetch_row.resultCode() == SQLITE_DONE);

  fetch_row = db::query<select_query>(2);
  assert(fetch_row.resultCode() == SQLITE_DONE);

  try {
    db::TransactionGuard txn;

    db::query<insert_query>(1, sqlite::blob{"hello"});
    db::query<insert_query>(2, sqlite::blob_view{"goodbye"});
  } catch (const std::exception &e) { }

  fetch_row = db::query<select_query>(1);
  assert(fetch_row.resultCode() == SQLITE_ROW);

  fetch_row = db::query<select_query>(2);
  assert(fetch_row.resultCode() == SQLITE_ROW);
}

int main(void)
{
  static const char create_table_query[] = "create table test (a, b)";
  db::query<create_table_query>();

  test_blob_and_blob_view();
  db::query<clear_table_query>();

  test_transactions();
  db::query<clear_table_query>();
}
