#ifndef RUNEFLYFF_MOB_H
#define RUNEFLYFF_MOB_H
#pragma once

namespace rune {
    namespace model {
        namespace spawn {
            struct MobSpawn {
            protected:
                long long lastrespawn;

            public:
                const int type;
                const int normal;
                const int agro;
                const float x;
                const float y;
                const float z;
                const int rx;
                const int ry;
                const int mapid;
                const int respawnrate;

                MobSpawn(int type, int normal, int agro, float x, float y, float z, int rx, int ry, int mapid, int respawnrate, int lastrespawn) :
                    type(type),
                    normal(normal),
                    agro(agro),
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

#endif //RUNEFLYFF_MOB_H
