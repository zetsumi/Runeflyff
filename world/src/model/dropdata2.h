#ifndef RUNEFLYFF_DROPDATA2_H
#define RUNEFLYFF_DROPDATA2_H
#pragma once

namespace rune {
    namespace model {
        struct DropData2 {
            const int id;
            const float dropchance;
            const int nmin, nmax;

            DropData2(int id, float dropchance, int min, int max) :
                id(id),
                dropchance(dropchance),
                nmin(min),
                nmax(max) {}
        };
    }
}

#endif //RUNEFLYFF_DROPDATA2_H
