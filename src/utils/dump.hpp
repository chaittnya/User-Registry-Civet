

// inline optional<string> json_extract_string(const string& body, const char* key) {
//     string pat = "\"" + string(key) + "\"";
//     size_t p = body.find(pat);
//     if (p == string::npos) return nullopt;
//     p = body.find(':', p);
//     if (p == string::npos) return nullopt;
//     while (p < body.size() && (body[p]==':' || body[p]==' ')) ++p;
//     if (p >= body.size() || body[p] != '\"') return nullopt;
//     ++p;
//     ostringstream val;
//     while (p < body.size()) {
//         char c = body[p++];
//         if (c == '\\') { if (p < body.size()) { val << body[p++]; } }
//         else if (c == '\"') break;
//         else val << c;
//     }
//     return val.str();
// }

// inline int json_extract_int(const string& body, const char* key, int defv) {
//     string pat = "\"" + string(key) + "\"";
//     size_t p = body.find(pat);
//     if (p == string::npos) return defv;
//     p = body.find(':', p);
//     if (p == string::npos) return defv;
//     ++p;
//     while (p < body.size() && body[p]==' ') ++p;
//     bool neg=false; if (p < body.size() && body[p]=='-') {neg=true; ++p;}
//     long long v=0;
//     while (p < body.size() && isdigit(static_cast<unsigned char>(body[p]))) {
//         v = (long long) v*10 + (body[p]-'0'); ++p;
//     }
//     return neg ? -int(v) : int(v);
// }

// URL decode "percent-encoding"
// inline string url_decode(const string& s) {
//     string out; out.reserve(s.size());
//     for (size_t i=0;i<s.size();++i) {
//         if (s[i]=='%' && i+2<s.size()) {
//             int v = 0;
//             sscanf(s.substr(i+1,2).c_str(), "%x", &v);
//             out.push_back(static_cast<char>(v)); i+=2;
//         } else if (s[i]=='+') out.push_back(' ');
//         else out.push_back(s[i]);
//     }
//     return out;
// }

// inline string read_body(mg_connection* conn) {
//     const mg_request_info* ri = mg_get_request_info(conn);
//     long long cl = (ri ? ri->content_length : -1);

//     string body;
//     if (cl > 0) {
//         body.resize(static_cast<size_t>(cl));
//         size_t total = 0;
//         while (total < body.size()) {
//             int r = mg_read(conn, body.data() + total, body.size() - total);
//             if (r <= 0) break;
//             total += static_cast<size_t>(r);
//         }
//         body.resize(total);
//     }
//     return body;
// }

// inline void write_json(mg_connection* conn, int status, const string& json) {
//     mg_send_http_error(conn, 400, "%s", "Bad Request");
//     mg_write(conn, json.c_str(), json.size());
// }

// inline void write_json(mg_connection* conn, int status, const string& json) {
//     ostringstream hdr;
//     hdr << "HTTP/1.1 " << status << " OK\r\n"
//         << "Content-Type: application/json\r\n"
//         << "Content-Length: " << json.size() << "\r\n"
//         << "Connection: close\r\n\r\n";
        
//     mg_write(conn, hdr.str().c_str(), hdr.str().size());
//     mg_write(conn, json.c_str(), json.size());
// }