#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libyang/libyang.h>
#include <libnetconf2/netconf.h>
#include <sysrepo.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>

#ifdef DOCUMENTATION
Here is a single C++ file that combines everything required to create a NETCONF server using the libyang and libnetconf2 libraries.
This example assumes you have the necessary YANG model files and dependencies installed.

Explanation
Include Headers: Includes necessary headers for libyang, libnetconf2, and sysrepo.
Define Server Context: Defines a structure to hold the server context, including the libyang context and RPC callback.
RPC Callback: A simple callback function that handles incoming RPCs. This example just prints the RPC and returns an OK reply.
Initialize Server: Initializes the libyang context and loads the specified YANG models. Sets the RPC callback function.
Start the NETCONF Server: Initializes the libnetconf2 server, creates a listening endpoint, sets the RPC callback function, and starts listening for incoming NETCONF sessions.
Main Function: The main function initializes the server, loads the YANG models, and starts the NETCONF server.

Build and Run
Install Dependencies: Ensure you have installed libyang and libnetconf2.
Compile: Use gcc or another C compiler to compile the program. Make sure to link against libyang and libnetconf2.

gcc -o netconf_server netconf_server.c -lxml2 -lssh -lnetconf2 -lyang

Run: Run the server executable.
./netconf_server

#endif


// Define a structure to hold server context
struct server_context {
    struct ly_ctx *ly_ctx;
    struct nc_server_reply *(*get_rpc_callback)(struct nc_session *session, const struct nc_rpc *rpc, struct ly_ctx *ctx);
};

// RPC Callback function
struct nc_server_reply *get_rpc(struct nc_session *session, const struct nc_rpc *rpc, struct ly_ctx *ctx) {
    // This function should handle incoming RPCs
    // For simplicity, it just prints the RPC and returns an empty reply
    printf("Received RPC: %s\n", ly_ctx_get_module(ctx, "ietf-netconf", NULL)->name);
    return nc_server_reply_ok();
}

// Initialize the server context and load YANG models
int server_init(struct server_context *server_ctx, const char **yang_files, size_t yang_file_count) {
    // Initialize libyang context
    server_ctx->ly_ctx = ly_ctx_new(NULL, 0);
    if (!server_ctx->ly_ctx) {
        fprintf(stderr, "Failed to create libyang context\n");
        return EXIT_FAILURE;
    }

    // Load YANG models
    for (size_t i = 0; i < yang_file_count; ++i) {
        if (!ly_ctx_load_module(server_ctx->ly_ctx, yang_files[i], NULL)) {
            fprintf(stderr, "Failed to load YANG model: %s\n", yang_files[i]);
            return EXIT_FAILURE;
        }
    }

    // Set the RPC callback function
    server_ctx->get_rpc_callback = get_rpc;

    return EXIT_SUCCESS;
}

// Main function to start the NETCONF server
int main(int argc, char **argv) {
    struct server_context server_ctx;

    const char *yang_files[] = {
        "ietf-netconf.yang",
        "your-model.yang"
    };

    if (server_init(&server_ctx, yang_files, sizeof(yang_files) / sizeof(yang_files[0])) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    // Initialize libnetconf2 server
    nc_server_init(server_ctx.ly_ctx);

    // Create a listening endpoint
    if (nc_server_add_endpt("main", NC_TI_LIBSSH, 0) == NULL) {
        fprintf(stderr, "Failed to create NETCONF endpoint\n");
        return EXIT_FAILURE;
    }

    // Set the RPC callback
    nc_server_set_rpc_clb(server_ctx.get_rpc_callback);

    // Start listening for incoming NETCONF sessions
    nc_server_listen();

    // Clean up and close the server
    nc_server_destroy();
    ly_ctx_destroy(server_ctx.ly_ctx, NULL);

    return EXIT_SUCCESS;
}
