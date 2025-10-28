#include <CivetServer.h>
#include <thread>
#include <chrono>
#include <iostream>

#include "app/app.hpp"
#include "handlers/users_handler.hpp"

int main() {
    App app;

    if (!app.db.connect_from_env()) {
        std::cerr << "Failed to connect to Postgres. Set DATABASE_URL.\n";
        return 1;
    }
    if (!app.db.prepare()) {
        std::cerr << "Failed to prepare schema/statements.\n";
        return 1;
    }

    const char *options[] = {
        "listening_ports", "8080",
        "num_threads",     "8",
        nullptr
    };
    CivetServer server(options);

    UsersHandler users(&app);
    server.addHandler("/users", users);             // POST /users
    server.addHandler("/users/", users);            // GET  /users/{id}
    server.addHandler("/users/by-phone", users);    // GET  /users/by-phone?mobile=...
    server.addHandler("/token", users);             // POST /token
    server.addHandler("/healthz", users);           // GET  /healthz

    std::cout << "listening on http://localhost:8080\n";
    while (true) std::this_thread::sleep_for(std::chrono::hours(24));
}
