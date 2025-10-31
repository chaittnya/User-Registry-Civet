#include <CivetServer.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <cstdlib>

#include "app/app.hpp"
#include "db/db.hpp"
#include "handlers/users_handler.hpp"

using namespace std;

int main() {
    App app;

    if (!Pg::init_from_env()) { 
        cerr << "Failed to connect to Postgres. Set CIVET_DATABASE_URL.\n";
        return 1;
    }
    if (!Pg::instance().ensure_schema()) { 
        cerr << "Failed to prepare database." << endl;
        return 1;
    }

    const char* port = getenv("CIVET_PORT");
    const char* threads = getenv("CIVET_THREADS");
    
    const char *options[] = {
        "listening_ports", port && *port ? port : "8080",
        "num_threads", threads && *threads ? threads : "8",
        nullptr
    };
    CivetServer server(options);
    UsersHandler users(&app);
    server.addHandler("/users", users); // POST /users
    server.addHandler("/users/", users); // GET  /users/{id}
    server.addHandler("/users/by-phone", users); // GET  /users/by-phone?mobile=...
    server.addHandler("/token", users); // POST /token
    server.addHandler("/healthz", users); // GET  /healthz
    
    cout << "Started server with "<< threads <<" threads, listening on PORT " << port << endl;
    while (true) this_thread::sleep_for(chrono::hours(24));
}
