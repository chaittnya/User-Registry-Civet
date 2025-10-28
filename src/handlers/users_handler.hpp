#pragma once
#include <CivetServer.h>
#include "../app/app.hpp"

class UsersHandler : public CivetHandler {
    App* app_;
public:
    explicit UsersHandler(App* a): app_(a) {}
    bool handlePost(CivetServer*, mg_connection* conn) override;
    bool handleGet (CivetServer*, mg_connection* conn) override;
};
