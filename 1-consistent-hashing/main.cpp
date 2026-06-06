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
    // int server_count = std::getenv("INIT_SERVER_COUNT") ? std::stoi(std::getenv("INIT_SERVER_COUNT")) : 5;
    int virtual_node_count = std::getenv("VIRTUAL_NODES_PER_SERVER") ? std::stoi(std::getenv("VIRTUAL_NODES_PER_SERVER")) : 150;
    std::unordered_map<std::string, std::vector<uint32_t>> server_to_nodes;
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
    // take server count and ids > hash > initialize the ring
    void initialise(const std::vector<std::string> &servers)
    {
        for (const auto &server : servers)
        {
            add_server(server);
        }
    }

    // get server for a key
    std::string hash_key_to_server(const std::string &key)
    {
        if (ring.empty()) return ""; // need to fix: should not return empty string
        
        uint32_t key_hash = hash_function(key);
        auto it = ring.lower_bound(key_hash);
        

        if (it == ring.end())
        { // ring.end means we did not find anything wrap to the first one
            it = ring.begin();
        }

        return it->second;
    }

    // take server id, key --> return hased value


    // add a brand new server to the ring
    bool add_server(const std::string &serverid)
    {

        // add server
        auto it = server_to_nodes.find(serverid);
        if (it != server_to_nodes.end()) // server already there on the ring
        {
            return false;
        }
        else
        {
            server_to_nodes[serverid] = {}; // server not there, good, initialise the mapping to {}
        }

        for (int i = 0; i < virtual_node_count; i++)
        {
            std::string server = serverid + std::to_string(i);
            uint32_t server_hash = hash_function(server);
            ring[server_hash] = serverid;
            server_to_nodes[serverid].push_back(server_hash);
        }

         // redistribute --> currently out of scope
        return true;
    }

    // remove a server
    void remove_server(const std::string &server_id)
    {
        // 1. remove server from the ring
        auto it = server_to_nodes.find(server_id);
        if (it == server_to_nodes.end()) {
            return;
        }

        const auto& virtual_nodes = it->second;

        for (const auto &node : virtual_nodes)
        {
            ring.erase(node);
        }

        // 2. remove from server_to_nodes
        server_to_nodes.erase(server_id);

        // 3. redistribute --> currently out of scope
    }
};

int main()
{
    ServerManagement server_manager;
    std::vector<std::string> servers = {"server1", "server2", "server3", "server4"};
    server_manager.initialise(servers);
}

// few notes
// not handling collisions for now, in real systems a 64 bit hash is used
// that which makes collisions so astronomically unlikley that the complexity of collision handling is not worth it/