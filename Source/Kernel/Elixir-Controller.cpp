// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021 by Agustin Alvarez. All rights reserved.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Elixir-Controller.hpp"
#include <winhttp.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   DATA   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Elixir::Controller
{
    static std::unordered_map<uint32_t, Item> s_Database;

    static int32_t s_Round;
    static int32_t s_Left;
    static int32_t s_Right;
    static bool    s_Ready_1;
    static bool    s_Ready_2;
    static int32_t s_Energy_1;
    static int32_t s_Energy_2;
    static int32_t s_Energy_Round_1;
    static int32_t s_Energy_Round_2;
    static Cards   s_Active_Cards_1;
    static Cards   s_Active_Cards_2;
    static Cards   s_Hand_Cards_1;
    static Cards   s_Hand_Cards_2;
    static Cards   s_Secret_Cards_1;
    static Cards   s_Secret_Cards_2;
    static Cards   s_Discard_Cards_1;
    static Cards   s_Discard_Cards_2;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Elixir::Controller
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    constexpr uint32_t k_Max_Card_Per_Play = 2;

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    auto GetString(uint32_t Address)
    {
        uint32_t Length = * (uint32_t *) (Address + 0x08);

        return std::wstring((wchar_t *) (Address + 0x0C), Length);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    auto GetName(uint32_t BattleCardProcessorPtr, uint32_t Card, uint32_t Team)
    {
        static uint32_t s_Module = reinterpret_cast<uint32_t>(::GetModuleHandleW(L"GameAssembly.dll"));

        uint32_t CardTrackingInfoPtr
            = ((uint32_t (*)(uint32_t, uint32_t, int32_t)) (s_Module + 0x13A3300))(BattleCardProcessorPtr, Card, Team);

        return ((uint32_t (*)(uint32_t)) (s_Module + 0x131DF80))(CardTrackingInfoPtr);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    auto GetDisplay(const std::wstring& Name, uint32_t Position, uint32_t Team)
    {
        if (Team == 0)
        {
            switch (Position)
            {
            case 0: return L"CR." + Name;
            case 1: return L"TR." + Name;
            case 2: return L"BR." + Name;
            case 3: return L"CM." + Name;
            case 4: return L"TL." + Name;
            case 5: return L"BL." + Name;
            case 6: return L"CL." + Name;
            }
        }
        else
        {
            switch (Position)
            {
            case 0: return L"CL." + Name;
            case 1: return L"TL." + Name;
            case 2: return L"BL." + Name;
            case 3: return L"CM." + Name;
            case 4: return L"TR." + Name;
            case 5: return L"BR." + Name;
            case 6: return L"CR." + Name;
            }
        }
        return Name;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    auto GetStateListData(uint32_t Address, uint32_t Offset)
    {
        const uint32_t ArrayListPtr = * (uint32_t *) (* (uint32_t *) (Address + 0x08) + 0x08);

        return * (uint32_t *) (ArrayListPtr + Offset * 0x04 + 0x10);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    auto GetStateListLength(uint32_t Address)
    {
        const uint32_t ArrayListPtr = * (uint32_t *) (Address + 0x08);

        return * (uint32_t *) (ArrayListPtr + 0x0C);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Functor>
    auto GetStateListData(uint32_t Address, Functor && Function)
    {
        const uint32_t Length = GetStateListLength(Address);

        for (uint32_t Element = 0; Element < Length; ++Element)
        {
            uint32_t Object = GetStateListData(Address, Element);

            if (Object > 0)
            {
                Function(Object);
            }
        }
        return Length;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Flags Update(uint32_t CardBattleControllerPtr)
    {
        Flags Dirty { 0 };

        const uint32_t BattleConfigPtr            = * (uint32_t *) (CardBattleControllerPtr + 0x34);
        const uint32_t BaseBattleDataProcessorPtr = * (uint32_t *) (CardBattleControllerPtr + 0x44);
        const uint32_t BattleStatePtr             = * (uint32_t *) (BaseBattleDataProcessorPtr + 0x14);

        if (BattleConfigPtr == 0 || BattleStatePtr == 0)
        {
            s_Round = 0xDEADBEEF;
            return Dirty;
        }

        const uint32_t FirstTeamStatePtr  = * (uint32_t *) (BattleStatePtr + 0x0C);
        const uint32_t SecondTeamStatePtr = * (uint32_t *) (BattleStatePtr + 0x10);
        const uint32_t RoundStatePtr      = * (uint32_t *) (BattleStatePtr + 0x18);

        const uint32_t Turn = * (uint32_t *) (BattleStatePtr + 0x2C);
        s_Left  = * (uint32_t *) (BattleConfigPtr + 0x14);
        s_Right = (s_Left ? 0 : 1);

        if (s_Round != Turn)
        {
            s_Round    = Turn;
            s_Energy_1 = * (uint32_t *) (FirstTeamStatePtr  + 0x1C);
            s_Energy_2 = * (uint32_t *) (SecondTeamStatePtr + 0x1C);
            s_Energy_Round_1 = 0;
            s_Energy_Round_2 = 0;

        #ifdef _PRO_VERSION
            s_Database.clear();

            GetStateListData(* (uint32_t *) (FirstTeamStatePtr  + 0x0C), [&](uint32_t ElementFighterState)
            {
                GetStateListData(* (uint32_t *) (ElementFighterState + 0x2C), [&](uint32_t CardState)
                {
                    if (* (int32_t *) (ElementFighterState + 0x18) <= 0 &&
                        * (int32_t *) (ElementFighterState + 0x20) <= 0)
                    {
                        return;
                    }

                    Item Data;
                    Data.Fighter  = ElementFighterState;
                    Data.Team     = 0;
                    Data.Card     = CardState;

                    //Trace(L"P1. %d - %X", * (uint32_t *)(ElementFighterState+0x14), * (uint32_t *) (CardState + 0x0C));
                    s_Database.emplace(* (uint32_t *) (CardState + 0x0C), Data);
                });
            });

            GetStateListData(* (uint32_t *) (SecondTeamStatePtr + 0x0C), [&](uint32_t ElementFighterState)
            {
                GetStateListData(* (uint32_t *) (ElementFighterState + 0x2C), [&](uint32_t CardState)
                {
                    if (* (int32_t *) (ElementFighterState + 0x18) <= 0 &&
                        * (int32_t *) (ElementFighterState + 0x20) <= 0)
                    {
                        return;
                    }

                    Item Data;
                    Data.Fighter  = ElementFighterState;
                    Data.Team     = 1;
                    Data.Card     = CardState;

                    //Trace(L"P2. %d - %X", * (uint32_t *)(ElementFighterState+0x14), * (uint32_t *) (CardState + 0x0C));
                    s_Database.emplace(* (uint32_t *) (CardState + 0x0C), Data);
                });
            });

            s_Secret_Cards_1.clear();
            s_Secret_Cards_2.clear();

            GetStateListData(* (uint32_t *) (FirstTeamStatePtr  + 0x10), [&](uint32_t ID)
            {
                uint32_t Name = GetName(BaseBattleDataProcessorPtr, s_Database[ID].Card, 0);

                Card State;
                State.ID   = ID;
                State.Name = GetDisplay(GetString(Name), 0, 0);

                s_Secret_Cards_1.push_back(State);
            });

            GetStateListData(* (uint32_t *) (SecondTeamStatePtr + 0x10), [&](uint32_t ID)
            {
                uint32_t Name = GetName(BaseBattleDataProcessorPtr, s_Database[ID].Card, 1);

                Card State;
                State.ID   = ID;
                State.Name = GetDisplay(GetString(Name), 0, 1);

                s_Secret_Cards_2.push_back(State);
            });

            s_Discard_Cards_1.clear();
            s_Discard_Cards_2.clear();

            GetStateListData(* (uint32_t *) (FirstTeamStatePtr  + 0x18), [&](uint32_t ID)
            {
                uint32_t Name = GetName(BaseBattleDataProcessorPtr, s_Database[ID].Card, 0);

                Card State;
                State.ID   = ID;
                State.Name = GetDisplay(GetString(Name), 0, 0);

                s_Discard_Cards_1.push_back(State);
            });

            GetStateListData(* (uint32_t *) (SecondTeamStatePtr + 0x18), [&](uint32_t ID)
            {
                uint32_t Name = GetName(BaseBattleDataProcessorPtr, s_Database[ID].Card, 1);

                Card State;
                State.ID   = ID;
                State.Name = GetDisplay(GetString(Name), 0, 1);

                s_Discard_Cards_2.push_back(State);
            });

            Dirty.Secret_1 = true;
            Dirty.Secret_2 = true;
        #endif // _PRO_VERSION
        }
        else
        {
                s_Energy_Round_1 = * (uint32_t *) (FirstTeamStatePtr  + 0x1C) - s_Energy_1;
                s_Energy_Round_2 = * (uint32_t *) (SecondTeamStatePtr + 0x1C) - s_Energy_2;
        }

        s_Ready_1 = * (bool *) (RoundStatePtr + 0x14);
        s_Ready_2 = * (bool *) (RoundStatePtr + 0x1C);

    #ifdef _PRO_VERSION
        Cards Active_Cards_1;
        Cards Active_Cards_2;

        GetStateListData(* (uint32_t *) (RoundStatePtr  + 0x10), [&](uint32_t ElementCardState)
        {
            uint32_t Name = GetName(BaseBattleDataProcessorPtr, ElementCardState, 0);

            Card State;
            State.ID      = * (uint32_t *) (ElementCardState + 0x0C);
            State.Name    = GetDisplay(GetString(Name), -1, 0);
            State.Energy  = * (uint32_t *) (ElementCardState  + 0x14);
            State.Attack  = * (uint32_t *) (ElementCardState  + 0x1C);
            State.Defense = * (uint32_t *) (ElementCardState  + 0x20);

            Active_Cards_1.push_back(State);
        });

        GetStateListData(* (uint32_t *) (RoundStatePtr  + 0x18), [&](uint32_t ElementCardState)
        {
            uint32_t Name = GetName(BaseBattleDataProcessorPtr, ElementCardState, 1);

            Card State;
            State.ID      = * (uint32_t *) (ElementCardState + 0x0C);
            State.Name    = GetDisplay(GetString(Name), -1, 1);
            State.Energy  = * (uint32_t *) (ElementCardState  + 0x14);
            State.Attack  = * (uint32_t *) (ElementCardState  + 0x1C);
            State.Defense = * (uint32_t *) (ElementCardState  + 0x20);

            Active_Cards_2.push_back(State);
        });

        if ((Dirty.Active_1 = (s_Active_Cards_1 != Active_Cards_1)))
        {
            s_Active_Cards_1 = Active_Cards_1;
        }
        if ((Dirty.Active_2 = (s_Active_Cards_2 != Active_Cards_2)))
        {
            s_Active_Cards_2 = Active_Cards_2;
        }

        if (Dirty.Secret_1 || Dirty.Active_1)
        {
            std::unordered_map<uint32_t, uint32_t> Counter;

            for (const Card& Card : s_Secret_Cards_1)
            {
                ++ Counter[Card.ID];
            }

            for (const Card& Card : s_Discard_Cards_1)
            {
                ++ Counter[Card.ID];
            }

            for (const Card& Card : s_Active_Cards_1)
            {
                ++ Counter[Card.ID];
            }

            s_Hand_Cards_1.clear();

            for (const auto [Key, Item] : s_Database)
            {
                if (Item.Team == 1) continue;

                if (Counter[Key] != k_Max_Card_Per_Play)
                {
                    std::wstring Name = GetString(GetName(BaseBattleDataProcessorPtr, Item.Card, 0));

                    Card State;
                    State.ID    = Key;
                    State.Name  = GetDisplay(Name, Item.Position, 0);
                    State.Name += L" (" + std::to_wstring(k_Max_Card_Per_Play - Counter[Key]) + L")";

                    s_Hand_Cards_1.emplace_back(State);
                }
            }
        }

        if (Dirty.Secret_2 || Dirty.Active_2)
        {
            std::unordered_map<uint32_t, uint32_t> Counter;

            for (const Card& Card : s_Secret_Cards_2)
            {
                ++ Counter[Card.ID];
            }

            for (const Card& Card : s_Discard_Cards_2)
            {
                ++ Counter[Card.ID];
            }

            for (const Card& Card : s_Active_Cards_2)
            {
                ++ Counter[Card.ID];
            }

            s_Hand_Cards_2.clear();

            for (const auto [Key, Item] : s_Database)
            {
                if (Item.Team == 0) continue;

                if (Counter[Key] != k_Max_Card_Per_Play)
                {
                    std::wstring Name = GetString(GetName(BaseBattleDataProcessorPtr, Item.Card, 1));

                    Card State;
                    State.ID    = Key;
                    State.Name  = GetDisplay(Name, Item.Position, 1);
                    State.Name += L" (" + std::to_wstring(k_Max_Card_Per_Play - Counter[Key]) + L")";

                    s_Hand_Cards_2.emplace_back(State);
                }
            }
        }
    #endif // _PRO_VERSION

        return Dirty;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    int32_t GetTurn()
    {
        return s_Round;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    int32_t GetLeft()
    {
        return s_Left;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    int32_t GetRight()
    {
        return s_Right;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    int32_t GetEnergy(int32_t Team)
    {
        return (Team ? s_Energy_2 : s_Energy_1);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    int32_t GetDelta(int32_t Team)
    {
        return (Team ? s_Energy_Round_2 : s_Energy_Round_1);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    bool HasFinished(int32_t Team)
    {
        return (Team ? s_Ready_2 : s_Ready_1);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Cards & GetActiveCards(int32_t Team)
    {
        return (Team ? s_Active_Cards_2 : s_Active_Cards_1);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Cards & GetHandCards(int32_t Team)
    {
        return (Team ? s_Hand_Cards_2 : s_Hand_Cards_1);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Cards & GetSecretCards(int32_t Team)
    {
        return (Team ? s_Secret_Cards_2 : s_Secret_Cards_1);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Cards & GetDiscardCards(int32_t Team)
    {
        return (Team ? s_Discard_Cards_2 : s_Discard_Cards_1);
    }
}