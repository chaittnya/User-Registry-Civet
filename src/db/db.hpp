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
#include "../models/user.hpp"

class Pg {
    PGconn* conn_{nullptr};
public:
    ~Pg();
    bool connect_from_env();   // uses env var DATABASE_URL (PostgreSQL DSN)
    bool prepare();            // create schema + prepared statements
    bool exec_ok(const char* sql);

    std::optional<User> create_user(const std::string& name, const std::string& mobile);
    std::optional<User> get_user_by_id(const std::string& id);
    std::optional<User> get_user_by_mobile(const std::string& mobile);
};
