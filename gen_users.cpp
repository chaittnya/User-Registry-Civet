#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <random>
#include <sstream>

#if __has_include(<libpq-fe.h>)
#include <libpq-fe.h>
#elif __has_include(<postgresql/libpq-fe.h>)
#include <postgresql/libpq-fe.h>
#else
#error "libpq-fe.h not found. Install libpq-dev or postgresql-devel."
#endif

using namespace std;

string random_hex8(mt19937 &rng)
{
    static const char hex[] = "0123456789abcdef";
    uniform_int_distribution<int> dist(0, 15);
    string s(8, '0');
    for (auto &ch : s)
        ch = hex[dist(rng)];
    return s;
}

int main()
{
    const char *dsn = getenv("DATABASE_URL");
    if (!dsn)
    {
        cerr << "Please set DATABASE_URL environment variable.\n";
        return 1;
    }

    PGconn *conn = PQconnectdb(dsn);
    if (PQstatus(conn) != CONNECTION_OK)
    {
        cerr << "DB connection failed: " << PQerrorMessage(conn) << "\n";
        PQfinish(conn);
        return 1;
    }

    mt19937 rng(random_device{}());
    vector<pair<string, string>> users;
    users.reserve(2000);

    for (int i = 0; i < 2000; ++i)
    {
        ostringstream name, mobile;
        name << "user_" << random_hex8(rng);
        mobile << "" << (9000000000LL + i);
        users.emplace_back(name.str(), mobile.str());
    }

    PGresult *res = PQexec(conn, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        cerr << "BEGIN failed.\n";
        PQclear(res);
        PQfinish(conn);
        return 1;
    }
    PQclear(res);

    const char *stmtName = "ins_user_gen";
    res = PQprepare(conn, stmtName,
                    "INSERT INTO users(name, mobile) VALUES($1,$2)", 2, nullptr);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        cerr << "Prepare failed: " << PQerrorMessage(conn) << "\n";
        PQclear(res);
        PQfinish(conn);
        return 1;
    }
    PQclear(res);

    size_t inserted = 0;
    for (auto &u : users)
    {
        const char *params[2] = {u.first.c_str(), u.second.c_str()};
        res = PQexecPrepared(conn, stmtName, 2, params, nullptr, nullptr, 0);
        if (PQresultStatus(res) == PGRES_COMMAND_OK ||
            PQresultStatus(res) == PGRES_TUPLES_OK)
        {
            ++inserted;
        }
        else
        {
            cerr << "Insert failed for " << u.first << ": "
                      << PQerrorMessage(conn);
        }
        PQclear(res);
    }

    res = PQexec(conn, "COMMIT");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        cerr << "COMMIT failed: " << PQerrorMessage(conn) << "\n";
    PQclear(res);

    cout << "Inserted " << inserted << " users successfully.\n";
    PQfinish(conn);
    return 0;
}
