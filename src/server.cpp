#include <stdio.h>
#include <enet/enet.h>
#include <cstdint>
#include <cstring>
#include <vector>

// STUN message types
constexpr uint16_t STUN_BINDING_REQUEST  = 0x0001;
constexpr uint16_t STUN_BINDING_SUCCESS  = 0x0101;

// Magic cookie
constexpr uint32_t STUN_MAGIC_COOKIE = 0x2112A442;

#pragma pack(push, 1)
struct StunHeader {
    uint16_t type;
    uint16_t length;
    uint32_t magic;
    uint8_t transactionId[12];
};
#pragma pack(pop)

std::vector<ENetPeer*> clients;

void share_peer_info(ENetHost* host)
{
	int num_clients = clients.size();
	for (int peer_idx = 0; peer_idx < num_clients; peer_idx++)
	{
		for (int dst_idx = 0; dst_idx < num_clients; dst_idx++)
		{
			if (peer_idx != dst_idx)
			{
				// Send a message containing the src peer info
				ENetPeer* src_peer = clients[peer_idx];
				ENetPeer* dst_peer = clients[dst_idx];

				char ip[16];
				enet_address_get_host_ip(&src_peer->address, ip, sizeof(ip));

				char buffer[50];
				sprintf(
					buffer, 
					"Client: %i || Peer IP: %s || Peer Port: %u", 
					peer_idx,
					ip,
					src_peer->address.port
					);

				ENetPacket* out = enet_packet_create(
					buffer,
					strlen((const char*)buffer) + 1,
					ENET_PACKET_FLAG_UNSEQUENCED
				);
				enet_peer_send(dst_peer, 0, out);
			}
		}
		enet_host_flush(host);
	}
}

bool handle_packet(ENetPeer* peer, ENetPacket* packet)
{
    if (packet->dataLength < sizeof(StunHeader)) 
	{
        return false; // not STUN
    }

    StunHeader* req = reinterpret_cast<StunHeader*>(packet->data);

    uint16_t msgType = ntohs(req->type);
    uint32_t magic   = ntohl(req->magic);

    if (magic != STUN_MAGIC_COOKIE)
	{
        return false; // Not a STUN message
	}

    if (msgType != STUN_BINDING_REQUEST)
	{
        return false; // Not a Binding Request
	}

	// Save pointer to connected peer
	clients.push_back(peer);

    // --- Build the STUN Binding Success Response ---
    uint8_t buffer[sizeof(StunHeader)] = {};
    StunHeader* resp = reinterpret_cast<StunHeader*>(buffer);

    resp->type  = htons(STUN_BINDING_SUCCESS);
    resp->length = htons(0); // no attributes in this minimal example
    resp->magic = htonl(STUN_MAGIC_COOKIE);

    // Copy transaction ID back so client accepts the response
    memcpy(resp->transactionId, req->transactionId, 12);

    // Create ENet packet containing the raw STUN response
    ENetPacket* out = enet_packet_create(
        buffer,
        sizeof(StunHeader),
        // ENET_PACKET_FLAG_UNSEQUENCED // STUN is usually UDP-like
		ENET_PACKET_FLAG_UNSEQUENCED
    );

    enet_peer_send(peer, 0, out);
	return true;
}

int main(int argc, char** argv)
{
    if (enet_initialize() != 0) {
        printf("Failed to initialize ENet.\n");
        return 1;
    }

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 1738;

    ENetHost* server = enet_host_create(&address, 32, 1, 0, 0);
    if (!server) {
        printf("Failed to create ENet server.\n");
        return 1;
    }

    while (true) {
        ENetEvent event;
        while (enet_host_service(server, &event, 10) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
			{
                if (handle_packet(event.peer, event.packet))
				{
					const uint8_t client_idx = clients.size() - 1;
					int num_clients = clients.size();
					if (num_clients >= 2)
					{
						printf("> 2 Clients connected. Sharing peer info...\n");
						share_peer_info(server);
					}
				}
                enet_packet_destroy(event.packet);
			}
			break;
            default:
			break;
            }
        }
    }

    enet_host_destroy(server);
    enet_deinitialize();
}

