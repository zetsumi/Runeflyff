#ifndef RUNEFLYFF_ITEM_H
#define RUNEFLYFF_ITEM_H
#pragma once

namespace rune {
    namespace model {
        namespace spawn {
            struct ItemSpawn {
            protected:
                long long lastrespawn;

            public:
                const int type;
                const int num;
                const float x;
                const float y;
                const float z;
                const int rx;
                const int ry;
                const int mapid;
                const int respawnrate;

                ItemSpawn(int type, int num, float x, float y, float z, int rx, int ry, int mapid, int respawnrate, long long lastrespawn) :
                    type(type),
                    num(num),
                    x(x),
                    y(y),
                    z(z),
                    rx(rx),
                    ry(ry),
                    mapid(mapid),
                    respawnrate(respawnrate),
                    lastrespawn(lastrespawn) {}
            };
        }
    }
}

#endif //RUNEFLYFF_ITEM_H
