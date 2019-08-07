# SQLiteWrapper

SQLiteWrapper is an easy-to-use, lightweight and concurrency-friendly SQLite wrapper written in C++17.

## A Quick Taste

```C++	
static const char db_name[] = "example.db";
using db = sqlite::Database<db_name>;

// Prepare and execute a query with 2 parameters, binding the
// integer 29 to the first parameter and the string "M" to the
// second.
//
// This query retrieves all users whose age is exactly 29 or
// whose first initial is M.
static const char select_users_query[]
  = R"(select first_name, last_name, age, website
       from users where age = ?1 or substr(first_name, 1, 1) = ?2)";
db::QueryResult fetch_row = db::query<select_users_query>(29, "M");

// Step through the results returned by the query.
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
```

More examples can be found in the [examples](./examples/) directory.

## Features

1. Concise and intuitive interface.  Being a header-only library, it's a breeze
   to embed into your project.
2. Written with absolute efficiency in mind: all prepared statements made by
   the library are persisted and reused via RAII and templated static
   variables; all text/blob parameter values are bound using the
   `SQLITE_STATIC` flag to avoid copying; and database connections are
   implicitly thread-local so that SQLites's connection-based locking can be
   disabled.  As much is computed at compile time as is feasibly possible in
   C++
3. Extensible -- aside from the native support for binding standard library
   types such as `std::string`, `std::string_view` and `std::optional<T>`,
   users can define their own custom serialization and deserialization hooks to
   be able to bind to and from other data types.
4. Particularly easy to use in multi-threaded applications.  Database
   connections are implicitly thread-local and are automatically created and
   destroyed, and a concurrency-friendly `sqlite3_busy_handler` is installed so
   that the dreadful `SQLITE_BUSY` return code is never seen.

## Caveats

1. Requires a C++ compiler that supports C++17.  Such compilers include `g++ 9` and `clang++ 8`.
2. Connections to the database are managed automatically for you.  If you find
   yourself needing more fine-grained control of when to connect and disconnect
   from your SQLite database, then this wrapper may currently not work well for you.
   (This is a deficiency of the wrapper which may be alleviated in the future.)

## How do I ...

### ... specify the database to connect to?

The database used is specified as a template argument to the `sqlite::Database`
class template.  The convention we adopted is to define a type alias for
`sqlite::Database<FOO>`, and to invoke methods via that type alias.

When the path to the database is known at compile-time, one can supply a path string
as the template argument:
```C++
inline const char db_name[] = "path/to/my_database.db";
using db = sqlite::Database<db_name>;
```

When the path to the database is not known at compile-time, one can instead
pass in a function which computes the database path:
```C++
std::string db_name(void) {
  std::string path;
  path.append(folder_name());
  path.append("database.db");
  return path;
}
using db = sqlite::Database<db_name>;
```
This function gets invoked when the first connection to the database is made in
the program.

### ... make sure the same set of pragmas is set on each connection?

By assigning the `post_connection_hook` appropriately:
```C++
using db = sqlite::Database<db_name>;
db::post_connection_hook = [] (sqlite3 *db_handle) {
  sqlite3_exec(db_handle, "pragma temp_store = memory",
               nullptr, nullptr, nullptr);
}
```
The `post_connection_hook` is called immediately after a connection is
successfully made.


### ... use a dynamically-generated query string?

The template argument to the `query` method can either be a string object or a
function or lambda that returns a string object.  If you need to dynamically
generate a query string, supply a function as a template argument like so:
```C++
std::string my_select_query(void) {
  std::string query = "select a, b, c from table where ";
  query.append(get_column_name());
  query.append(" = ?1");
  return query;
}

using db = sqlite::Database<db_name>;
auto fetch_row = db::query<my_select_query>(5);
...
```

### ... bind a BLOB parameter?

Use the `sqlite::blob` and `sqlite::blob_view` data types, which are BLOB
analogues of `std::string` and `std::string_view`:
```C++
db::query<my_insert_query>(sqlite::blob{"hello"});
```
If you want to extract a BLOB value from a column, pretend it is a TEXT value and
pass in an `std::string` or `std::string_view` object to the row fetcher.
