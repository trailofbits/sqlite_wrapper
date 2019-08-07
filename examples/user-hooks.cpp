#include "SQLiteWrapper.h"
#include <google/protobuf/message.h>

// This example illustrates how to create user-defined serialization and
// deserialization hooks for `google::protobuf::Message`s.

template <typename T>
using is_a_protobuf_message
    = std::enable_if_t<std::is_base_of_v<google::protobuf::Message, T>>;

// Define the serialization hook.
template <typename T>
constexpr auto sqlite::user_serialize_fn<T, is_a_protobuf_message<T>> =
  [] (const google::protobuf::Message &message) {
    return sqlite::blob{message.SerializeAsString()};
  };

// Define the deserialization hook.
template <typename T>
constexpr auto sqlite::user_deserialize_fn<T, is_a_protobuf_message<T>> =
  [] (sqlite::blob_view serialized_message) {
    T message;
    message.ParseFromArray(&serialized_message[0],
                           serialized_message.length());
    return message;
  };

static const char db_name[] = ":memory:";
using db = sqlite::Database<db_name>;

void example (const std::string &name,
              const google::protobuf::Message &new_message) {
  static const char insert_query[]
    = R"(insert into my_table (my_name, my_column) values (?1, ?2))";
  db::query<insert_query>(name, new_message);
}
