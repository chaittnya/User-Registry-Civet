# User-Registry-Civet
 The User Registry is a web service developed in C++ and CivetWeb library to manage user
 information like name, mobile number. The objective of this project is to design a webserver
 that can handle multiple types of client requests, some served from in-memory LRU cache and
 others that require database access. It uses PostgreSQL database for persistent storage and
 CMake as build tool.

 The system is designed to work efficiently by processing multiple user requests
 at the same time using multi-threading. It also uses a database, cache to manage data and
 token generator.
