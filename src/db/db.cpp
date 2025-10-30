#include "db.hpp"
#include<iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>

using namespace std;

static string g_conninfo;

bool Pg::init_from_env() {
  const char* dsn = getenv("DATABASE_URL");
  if (!dsn || !*dsn) {
    return false;
  }
  g_conninfo = dsn; 
  return true;
}
void Pg::shutdown() {}

thread_local Pg* tl_pg = nullptr;

Pg& Pg::instance() {
  if (!tl_pg) {
    tl_pg = new Pg();
  }
  return *tl_pg;
}

Pg::Pg() : conn(nullptr) {
    cout<<"Creating Connection"<<endl;
}

Pg::~Pg() { 
    cout<<"Closing Connection"<<endl;
    if (conn) {
        PQfinish(conn); 
        conn = nullptr; 
    } 
}

PGconn* Pg::get_connection() {
    if (conn && PQstatus(conn) == CONNECTION_OK) {
        return conn;
    }
    if (conn) { PQfinish(conn); conn = nullptr; }
    conn = PQconnectdb(g_conninfo.c_str());
    return (conn && PQstatus(conn) == CONNECTION_OK) ? conn : nullptr;
}

bool Pg::ensure_schema() {
    PGconn* c = get_connection(); 
    if (!c) {
        return false;
    }
    const char* ddl = "CREATE TABLE IF NOT EXISTS users (id BIGSERIAL PRIMARY KEY, name TEXT NOT NULL, mobile TEXT UNIQUE NOT NULL);";
    PGresult* r = PQexec(c, ddl);
    if (!r || PQresultStatus(r) != PGRES_COMMAND_OK) { 
        if (r) PQclear(r); 
        return false; 
    }
    PQclear(r); return true;
}

static optional<User> row_to_user(PGresult* res, int i) {
    User u; 
    u.id=PQgetvalue(res,i,0); 
    u.name=PQgetvalue(res,i,1); 
    u.mobile=PQgetvalue(res,i,2); 
    return u;
}

optional<User> Pg::get_user_by_id(const string& id) {
    PGconn* c = get_connection(); 
    if (!c) {
        return nullopt;
    }
    const char* sql = "SELECT id,name,mobile FROM users WHERE id = $1::bigint";
    const char* params[1] = { id.c_str() };
    PGresult* r = PQexecParams(c, sql, 1, nullptr, params, nullptr, nullptr, 0);
    if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) { 
        if (r) PQclear(r); 
        return nullopt; 
    }
    auto out = (PQntuples(r)>0) ? row_to_user(r,0) : optional<User>{};
    PQclear(r); 
    return out;
}

optional<User> Pg::get_user_by_mobile(const string& mobile) {
    PGconn* c = get_connection(); 
    if (!c) {
        return nullopt;
    }
    const char* sql = "SELECT id,name,mobile FROM users WHERE mobile = $1";
    const char* params[1] = { mobile.c_str() };
    PGresult* r = PQexecParams(c, sql, 1, nullptr, params, nullptr, nullptr, 0);
    if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) { 
        if (r) PQclear(r); 
        return nullopt; 
    }
    auto out = (PQntuples(r)>0) ? row_to_user(r,0) : optional<User>{};
    PQclear(r); 
    return out;
}

optional<User> Pg::create_user(const string& name, const string& mobile) {
    PGconn* c = get_connection(); 
    if (!c) {
        return nullopt;
    }
    const char* sql = "INSERT INTO users(name,mobile) VALUES($1,$2) ON CONFLICT (mobile) DO UPDATE SET name=EXCLUDED.name RETURNING id,name,mobile";
    const char* params[2] = { name.c_str(), mobile.c_str() };
    PGresult* r = PQexecParams(c, sql, 2, nullptr, params, nullptr, nullptr, 0);
    if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) { 
        if (r) PQclear(r); 
        return nullopt; 
    }
    auto out = (PQntuples(r)>0) ? row_to_user(r,0) : optional<User>{};
    PQclear(r); 
    return out;
}

vector<string> Pg::get_random_user_ids(int n) {
    vector<string> out;
    if (n <= 0) {
        return out;
    }
    PGconn* c = get_connection();
    if (!c) {
        return out;
    }

    const string lim = to_string(n);
    const char* params[1] = { lim.c_str() };
    static const char* sql = "SELECT id::text FROM users ORDER BY random() LIMIT $1::int";

    PGresult* r = PQexecParams(c, sql, 1, nullptr, params, nullptr, nullptr, 0);
    if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) {
        if (r) PQclear(r);
        return out;
    }
    const int rows = PQntuples(r);
    out.reserve(rows);
    for (int i = 0; i < rows; ++i) {
        out.emplace_back(PQgetvalue(r, i, 0));
    }
    PQclear(r);
    return out;
}