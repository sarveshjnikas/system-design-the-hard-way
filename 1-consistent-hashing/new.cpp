#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <optional>
#include <cstdlib>
#include <cstdint>
#include "MurmurHash3.h"

class ServerManagement
{
private:
    std::map<uint32_t, std::string> ring;
    int virtual_node_count;
    std::unordered_map<std::string, std::vector<uint32_t>> server_to_nodes;

    // PRIVATE — implementation detail
    std::uint32_t hash_function(const std::string &key)
    {
        uint32_t hash;
        MurmurHash3_x86_32(
            key.data(),
            key.size(),
            0,
            &hash);
        return hash;
    }

public:
    ServerManagement()
    {
        // FIX 4: guard stoi against invalid env var
        const char* env = std::getenv("VIRTUAL_NODES_PER_SERVER");
        if (env) {
            try {
                virtual_node_count = std::stoi(env);
            } catch (...) {
                virtual_node_count = 150;
            }
        } else {
            virtual_node_count = 150;
        }
    }

    void initialize(const std::vector<std::string> &servers)
    {
        for (const auto &server : servers)
        {
            add_server(server);
        }
    }

    // FIX 2: return optional instead of ""
    std::optional<std::string> hash_key_to_server(const std::string &key)
    {
        if (ring.empty())
        {
            return std::nullopt;
        }

        uint32_t key_hash = hash_function(key);
        auto it = ring.lower_bound(key_hash);

        if (it == ring.end())
        {
            it = ring.begin();
        }

        return it->second;
    }

    bool add_server(const std::string &serverid)
    {
        auto it = server_to_nodes.find(serverid);
        if (it != server_to_nodes.end())
        {
            return false;
        }

        server_to_nodes[serverid] = {};

        for (int i = 0; i < virtual_node_count; i++)
        {
            // FIX 1: "#" separator prevents "server1" + "2" == "server12" + ""
            std::string vnode_key = serverid + "#" + std::to_string(i);
            uint32_t server_hash = hash_function(vnode_key);

            // FIX 3: skip on collision, don't silently overwrite
            if (ring.find(server_hash) == ring.end())
            {
                ring[server_hash] = serverid;
                server_to_nodes[serverid].push_back(server_hash);
            }
        }

        return true;
    }

    void remove_server(const std::string &server_id)
    {
        auto it = server_to_nodes.find(server_id);
        if (it == server_to_nodes.end())
        {
            return;
        }

        for (const auto &node : it->second)
        {
            ring.erase(node);
        }

        server_to_nodes.erase(server_id);
    }
};

int main()
{
    ServerManagement server_manager;
    std::vector<std::string> servers = {"server1", "server2", "server3", "server4"};
    server_manager.initialize(servers);

    std::vector<std::string> keys = {"user_1", "user_2", "order_99"};

    for (const auto& key : keys)
    {
        auto server = server_manager.hash_key_to_server(key);
        std::cout << key << " -> " << (server ? *server : "no server") << "\n";
    }

    server_manager.remove_server("server2");
    std::cout << "\nAfter removing server2:\n";

    for (const auto& key : keys)
    {
        auto server = server_manager.hash_key_to_server(key);
        std::cout << key << " -> " << (server ? *server : "no server") << "\n";
    }
}