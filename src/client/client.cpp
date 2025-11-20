#include <enet/enet.h>
#include <iostream>

int main(int argc, char** argv) {
    if (enet_initialize() != 0) {
        std::cerr << "Failed to initialize ENet.\n";
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    // Create a client host
    ENetHost* client = enet_host_create(
        nullptr,   // no address (client)
        1,         // only one connection allowed
        2,         // two channels
        0, 0       // no bandwidth throttling
    );

    if (!client) {
        std::cerr << "Failed to create ENet client.\n";
        return EXIT_FAILURE;
    }

    ENetAddress address;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 3478;

    // Attempt to connect
    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);
    if (!peer) {
        std::cerr << "Failed to initiate connection.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Connecting...\n";

    // Wait up to 5 seconds for the server to respond
    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        std::cout << "Connected to server!\n";

        // Send "Hello World" to server
        const char* msg = "Hello World from ENet!";
        ENetPacket* packet = enet_packet_create(
            msg,
            strlen(msg) + 1,
            ENET_PACKET_FLAG_RELIABLE
        );
        enet_peer_send(peer, 0, packet);
        enet_host_flush(client);
    } else {
        enet_peer_reset(peer);
        std::cerr << "Connection timed out.\n";
        return EXIT_FAILURE;
    }

    // Listen for a reply for up to 3 seconds
    bool received = false;
    while (enet_host_service(client, &event, 3000) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                std::cout << "Received from server: "
                          << (char*)event.packet->data << "\n";
                enet_packet_destroy(event.packet);
                received = true;
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << "Disconnected.\n";
                break;

            default:
                break;
        }
        if (received) break;
    }

    // Clean up
    enet_peer_disconnect(peer, 0);

    while (enet_host_service(client, &event, 3000) > 0) {
        if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            std::cout << "Disconnected cleanly.\n";
            break;
        }
    }

    enet_host_destroy(client);
    return 0;
}

