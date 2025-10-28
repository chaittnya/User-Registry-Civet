#include "db.hpp"
#include<iostream>
#include <cstdlib>
#include <string>

Pg::~Pg() {
    if (conn_) PQfinish(conn_);
}

bool Pg::connect_from_env() {
    std::cout<<"connecting to database...\n";
    const char* dsn = std::getenv("DATABASE_URL");
    std::cout<<dsn<<std::endl;
    if (!dsn || !*dsn) return false;
    conn_ = PQconnectdb(dsn);
    return PQstatus(conn_) == CONNECTION_OK;
}

bool Pg::exec_ok(const char* sql) {
    PGresult* r = PQexec(conn_, sql);
    if (!r) return false;
    bool ok = (PQresultStatus(r) == PGRES_COMMAND_OK);
    PQclear(r);
    return ok;
}

bool Pg::prepare() {
    // Schema
    const char* schema =
        "CREATE TABLE IF NOT EXISTS users ("
        "  id BIGSERIAL PRIMARY KEY,"
        "  name TEXT NOT NULL,"
        "  mobile TEXT NOT NULL,"
        "  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()"
        ");";
    if (!exec_ok(schema)) return false;

    PGresult* r = nullptr;

    // Insert (upsert-like via DO NOTHING) and return row
    r = PQprepare(conn_, "ins_user",
        "INSERT INTO users(name,mobile) VALUES($1,$2) "
        "RETURNING id::text,name,mobile,TO_CHAR(created_at,'YYYY-MM-DD\"T\"HH24:MI:SSZ')",
        2, nullptr);
    if (!r || PQresultStatus(r) != PGRES_COMMAND_OK) { if (r) PQclear(r); return false; }
    PQclear(r);

    // Select by id
    r = PQprepare(conn_, "sel_user_id",
        "SELECT id::text,name,mobile,TO_CHAR(created_at,'YYYY-MM-DD\"T\"HH24:MI:SSZ') "
        "FROM users WHERE id=$1",
        1, nullptr);
    if (!r || PQresultStatus(r) != PGRES_COMMAND_OK) { if (r) PQclear(r); return false; }
    PQclear(r);

    // Select by mobile
    r = PQprepare(conn_, "sel_user_mobile",
        "SELECT id::text,name,mobile,TO_CHAR(created_at,'YYYY-MM-DD\"T\"HH24:MI:SSZ') "
        "FROM users WHERE mobile=$1",
        1, nullptr);
    if (!r || PQresultStatus(r) != PGRES_COMMAND_OK) { if (r) PQclear(r); return false; }
    PQclear(r);

    return true;
}

std::optional<User> Pg::create_user(const std::string& name, const std::string& mobile) {
    const char* params[2] = { name.c_str(), mobile.c_str() };
    PGresult* r = PQexecPrepared(conn_, "ins_user", 2, params, nullptr, nullptr, 0);
    if (!r) return std::nullopt;

    if (PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) == 1) {
        User u{ PQgetvalue(r,0,0), PQgetvalue(r,0,1), PQgetvalue(r,0,2), PQgetvalue(r,0,3) };
        PQclear(r);
        return u;
    }
    PQclear(r);
    return std::nullopt;
}

std::optional<User> Pg::get_user_by_id(const std::string& id) {
    const char* params[1] = { id.c_str() };
    PGresult* r = PQexecPrepared(conn_, "sel_user_id", 1, params, nullptr, nullptr, 0);
    if (!r) return std::nullopt;

    if (PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) == 1) {
        User u{ PQgetvalue(r,0,0), PQgetvalue(r,0,1), PQgetvalue(r,0,2), PQgetvalue(r,0,3) };
        PQclear(r);
        return u;
    }
    PQclear(r);
    return std::nullopt;
}

std::optional<User> Pg::get_user_by_mobile(const std::string& mobile) {
    const char* params[1] = { mobile.c_str() };
    PGresult* r = PQexecPrepared(conn_, "sel_user_mobile", 1, params, nullptr, nullptr, 0);
    if (!r) return std::nullopt;

    if (PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) == 1) {
        User u{ PQgetvalue(r,0,0), PQgetvalue(r,0,1), PQgetvalue(r,0,2), PQgetvalue(r,0,3) };
        PQclear(r);
        return u;
    }
    PQclear(r);
    return std::nullopt;
}

// std::optional<User> Pg::create_user(const std::string& id, const std::string& name, const std::string& mobile) {
//     const char* params[2] = {  name.c_str(), mobile.c_str() };
//     PGresult* r = PQexecPrepared(conn_, "ins_user", 3, params, nullptr, nullptr, 0);
//     if (!r) return std::nullopt;

//     auto status = PQresultStatus(r);
//     if (status == PGRES_TUPLES_OK && PQntuples(r) == 1) {
//         User u{ PQgetvalue(r,0,0), PQgetvalue(r,0,1), PQgetvalue(r,0,2), PQgetvalue(r,0,3) };
//         PQclear(r);
//         return u;
//     }
//     PQclear(r);
//     // Insert did nothing (id exists) → fetch existing row
//     return get_user_by_id(id);
// }

// std::optional<User> Pg::get_user_by_id(const std::string& id) {
//     const char* params[1] = { id.c_str() };
//     PGresult* r = PQexecPrepared(conn_, "sel_user_id", 1, params, nullptr, nullptr, 0);
//     if (!r) return std::nullopt;

//     if (PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) == 1) {
//         User u{ PQgetvalue(r,0,0), PQgetvalue(r,0,1), PQgetvalue(r,0,2), PQgetvalue(r,0,3) };
//         PQclear(r);
//         return u;
//     }
//     PQclear(r);
//     return std::nullopt;
// }

// std::optional<User> Pg::get_user_by_mobile(const std::string& mobile) {
//     const char* params[1] = { mobile.c_str() };
//     PGresult* r = PQexecPrepared(conn_, "sel_user_mobile", 1, params, nullptr, nullptr, 0);
//     if (!r) return std::nullopt;

//     if (PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) == 1) {
//         User u{ PQgetvalue(r,0,0), PQgetvalue(r,0,1), PQgetvalue(r,0,2), PQgetvalue(r,0,3) };
//         PQclear(r);
//         return u;
//     }
//     PQclear(r);
//     return std::nullopt;
// }
