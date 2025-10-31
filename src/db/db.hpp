#pragma once
#if __has_include(<libpq-fe.h>)
#include <libpq-fe.h>
#elif __has_include(<postgresql/libpq-fe.h>)
#include <postgresql/libpq-fe.h>
#else
#error "libpq-fe.h not found. Install libpq-dev (Debian/Ubuntu) or postgresql-devel (Fedora) and ensure headers are in the include path."
#endif
#include <optional>
#include <string>
#include <vector>
#include "../models/user.hpp"

using namespace std;

class Pg
{
public:
  static bool init_from_env();
  static Pg &instance();
  static void shutdown();

  bool ensure_schema();

  optional<User> get_user_by_id(const string &id);
  optional<User> get_user_by_mobile(const string &mobile);
  optional<User> create_user(const string &name, const string &mobile);
  vector<string> get_random_user_ids(int n);

  ~Pg();

private:
  Pg();
  PGconn *conn{nullptr};
  PGconn *get_connection();
};
