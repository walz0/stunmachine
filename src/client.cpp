#include <enet/enet.h>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <random>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <arpa/inet.h>   // for htons, ntohs, htonl, ntohl
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

constexpr uint16_t STUN_SERVER_PORT = 1738;
constexpr char* STUN_SERVER_ADDR = "127.0.0.1";

// STUN message types
constexpr uint16_t STUN_BINDING_REQUEST = 0x0001;
constexpr uint16_t STUN_BINDING_SUCCESS = 0x0101;
constexpr uint32_t STUN_MAGIC_COOKIE = 0x2112A442;

#pragma pack(push, 1)
struct StunHeader {
    uint16_t type;
    uint16_t length;
    uint32_t magic;
    uint8_t transactionId[12];
};
#pragma pack(pop)

void generate_transaction_id(uint8_t* id) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (int i = 0; i < 12; ++i) {
        id[i] = static_cast<uint8_t>(dis(gen));
    }
}

ENetPacket* handle_stun_packet(ENetPacket* packet)
{
	if (packet->dataLength >= sizeof(StunHeader)) 
	{
		StunHeader* resp = reinterpret_cast<StunHeader*>(packet->data);
		uint16_t msgType = ntohs(resp->type);
		uint32_t magic = ntohl(resp->magic);
		
		if (magic == STUN_MAGIC_COOKIE && msgType == STUN_BINDING_SUCCESS) 
		{
			return packet;
		} 
	}
	return nullptr;
}

StunHeader* get_stun_header(ENetPacket* packet)
{
	return reinterpret_cast<StunHeader*>(packet->data);
}

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
    // enet_address_set_host(&address, "31.220.109.234");
    enet_address_set_host(&address, STUN_SERVER_ADDR);
    address.port = STUN_SERVER_PORT;

    // Attempt to connect
    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);
    if (!peer) {
        std::cerr << "Failed to initiate connection.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Connecting...\n";

    // Wait up to 5 seconds for the server to respond
    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
    {
        std::cout << "Connected to server!\n";
        
        // --- Build a proper STUN Binding Request ---
        uint8_t buffer[sizeof(StunHeader)] = {};
        StunHeader* req = reinterpret_cast<StunHeader*>(buffer);
        
        req->type = htons(STUN_BINDING_REQUEST);
        req->length = htons(0);  // no attributes for minimal request
        req->magic = htonl(STUN_MAGIC_COOKIE);
        generate_transaction_id(req->transactionId);
        
        std::cout << "Sending STUN Binding Request...\n";
        
        // Create packet with the raw STUN data
        ENetPacket* packet = enet_packet_create(
            buffer,
            sizeof(StunHeader),
            ENET_PACKET_FLAG_RELIABLE
        );
        
        enet_peer_send(peer, 0, packet);
        enet_host_flush(client);
    } else {
        enet_peer_reset(peer);
        std::cerr << "Connection timed out.\n";
        return EXIT_FAILURE;
    }

    bool received = false;
	int32_t timeout_ms = 30000;
    while (enet_host_service(client, &event, timeout_ms) > 0) 
	{
        switch (event.type) 
		{
            case ENET_EVENT_TYPE_RECEIVE: 
			{
				// Handle STUN messages
				if (ENetPacket* stun_packet = handle_stun_packet(event.packet))
				{
					// Print STUN header
					StunHeader* header = get_stun_header(stun_packet);
					printf("\nReceived STUN Message:\n");
					printf("Type: %i\n", header->type);
					printf("Length: %i\n", header->length);
					printf("Magic Cookie: %08x\n", header->magic);
					printf("Transaction ID: ");
					for (int i = 0; i < 12; i++)
					{
						printf("%02x", header->transactionId[i]);
					}
					printf("\n");
				}
				else 
				{
					printf("\nReceived Message:\n");
					printf("%s\n", event.packet->data);
				}

                enet_packet_destroy(event.packet);
                received = true;
            }
			break;
            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << "Disconnected.\n";
			break;
            default:
			break;
        }
        // if (received) break;
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
