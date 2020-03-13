#include <libwebsockets.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <pqxx/pqxx>

static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_HTTP:
            lws_serve_http_file(wsi, "example.html", "text/html", NULL, 0);
            break;
        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] =
    {
        /* The first protocol must always be the HTTP handler */
        {
            "http-only",   /* name */
            callback_http, /* callback */
            0,             /* No per session data. */
            0,             /* max frame size / rx buffer */
        },
        {NULL, NULL, 0, 0} /* terminator */
};

int main(int argc, char *argv[]) {
    pqxx::connection conn{"dbname=test"};
    pqxx::work T{conn};
    pqxx::result R{T.exec("select * from test")};
    for (auto &&c : R) {
        std::cout << c[0].as(std::string()) << std::endl;
    }
    T.commit();
    conn.disconnect();

    return 0;

    lws_set_log_level(LLL_COUNT, NULL);

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));

    info.port = 8000;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    struct lws_context *context = lws_create_context(&info);

    while (1) {
        lws_service(context, /* timeout_ms = */ 1000000);
    }

    lws_context_destroy(context);

    return 0;
}