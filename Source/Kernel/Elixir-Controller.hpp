// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021 by Agustin Alvarez. All rights reserved.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Elixir-Memory.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Elixir::Controller
{
    // -=(Undocumented)=-
    struct Flags
    {
        uint32_t Secret_1 : 1;
        uint32_t Secret_2 : 1;
        uint32_t Active_1 : 1;
        uint32_t Active_2 : 1;
    };

    // -=(Undocumented)=-
    struct Card
    {
        int32_t      ID;
        std::wstring Name;
        int32_t      Energy;
        int32_t      Attack;
        int32_t      Defense;

        auto operator<=>(const Card &) const = default;
    };

    // -=(Undocumented)=-
    struct Item
    {
        uint32_t Fighter;
        uint32_t Team;
        uint32_t Card;
    };

    // -=(Undocumented)=-
    using Cards = std::vector<Card>;

    // -=(Undocumented)=-
    Flags Update(uint32_t CardBattleControllerPtr);

    // -=(Undocumented)=-
    int32_t GetTurn();

    // -=(Undocumented)=-
    int32_t GetLeft();

    // -=(Undocumented)=-
    int32_t GetRight();

    // -=(Undocumented)=-
    int32_t GetEnergy(int32_t Team);

    // -=(Undocumented)=-
    int32_t GetDelta(int32_t Team);

    // -=(Undocumented)=-
    bool HasFinished(int32_t Team);

    // -=(Undocumented)=-
    Cards & GetActiveCards(int32_t Team);

    // -=(Undocumented)=-
    Cards & GetHandCards(int32_t Team);

    // -=(Undocumented)=-
    Cards & GetSecretCards(int32_t Team);

    // -=(Undocumented)=-
    Cards & GetDiscardCards(int32_t Team);
}