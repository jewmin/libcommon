#ifndef __LIBCOMMON_PACKET_POOL_HPP__
#define __LIBCOMMON_PACKET_POOL_HPP__

#include "packet.hpp"
#include "object_pool.hpp"

class PacketPool : public ObjectPool<Packet, 512> {
public:
    void GC() {
        for (int i = free_list_.Count() - 1; i > -1; --i) {
            free_list_[i]->Free();
        }
    }
};

#endif