// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021 by Agustin Alvarez. All rights reserved.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Factory-Menu.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   DATA   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Elixir::Factory
{
    static Memory::Detour s_Detour;

#pragma section(".crt", read, write)
    __declspec(allocate(".crt")) static wchar_t LOADER_KEY[64];
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Elixir::Factory
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void __stdcall On_CardBattleControllerOnUpdate(uint32_t Instance)
    {
        Controller::Flags Flags = Controller::Update(Instance);

        const int32_t Left  = Controller::GetLeft();
        const int32_t Right = Controller::GetRight();

        Menu::Set_Player(Controller::GetEnergy(Right), Controller::GetDelta(Right), Controller::HasFinished(Right));

#ifdef _PRO_VERSION
        if (Flags.Secret_1)
        {
            Menu::Set_Secret(Left, Controller::GetSecretCards(0));
        }

        if (Flags.Active_1)
        {
            Menu::Set_Active(Left, Controller::GetActiveCards(0));
        }

        if (Flags.Secret_1 || Flags.Active_1)
        {
            Menu::Set_Hands(Left, Controller::GetHandCards(0));
        }
#endif // _PRO_VERSION

#ifdef _PRO_VERSION
        if (Flags.Secret_2)
        {
            Menu::Set_Secret(Right, Controller::GetSecretCards(1));
        }

        if (Flags.Active_2)
        {
            Menu::Set_Active(Right, Controller::GetActiveCards(1));
        }

        if (Flags.Secret_2 || Flags.Active_2)
        {
            Menu::Set_Hands(Right, Controller::GetHandCards(1));
        }
#endif // _PRO_VERSION
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    __declspec(naked) void __stdcall On_CardBattleControllerOnUpdate_Trampoline()
    {
        __asm PUSHAD
        __asm PUSHFD
        __asm PUSH DWORD PTR[ESP + 0x28]
        __asm CALL On_CardBattleControllerOnUpdate
        __asm POPFD
        __asm POPAD
        __asm JMP s_Detour.Destination
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void On_Game_Attach()
    {
        // Initialize Menu
        Elixir::Factory::Menu::Create();

        // Find GameAssembly.dll
        uint32_t Module = reinterpret_cast<uint32_t>(GetModuleHandleW(L"GameAssembly.dll"));

        Memory::Create_Detour(Module + 0xF42A70, 10, (uintptr_t) & On_CardBattleControllerOnUpdate_Trampoline, s_Detour);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void On_Game_Detach()
    {
        // Destroy Detour(s)
        Memory::Remove_Detour(s_Detour);

        // Destroy Overlay
        Elixir::Factory::Menu::Delete();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void On_Loader_Attach(uint32_t Instance)
    {
#ifdef _SHIPPING
        // Get GameAssembly.dll
        uint32_t Module = reinterpret_cast<uint32_t>(GetModuleHandleW(L"GameAssembly.dll"));

        // Get Address
        uint32_t Ronin_Key_Ptr = ((uint32_t (*)()) (Module + 0x1404400))();             // StateCollection
        Ronin_Key_Ptr = ((uint32_t (*)(uint32_t)) (Module + 0x143BEB0))(Ronin_Key_Ptr); // MyProfileData
        Ronin_Key_Ptr = * (uint32_t *) (Ronin_Key_Ptr + 0x10);                          // MyProfileData.email

        uint8_t * Address = (uint8_t *) LOADER_KEY;

        for (int i = 0; i < 128; ++i)
        {
            Address[i] ^= 0x32;
        }

        if (wcscmp(LOADER_KEY, (wchar_t *) (Ronin_Key_Ptr + 0x0C)) != 0)
        {
            return;
        }

#endif // _SHIPPING

        // Call Game's entry point
        On_Game_Attach();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void On_Loader_Detach()
    {
        On_Game_Detach();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    extern "C" bool __stdcall DllMain(uint32_t Instance, uint32_t Reason, uint32_t)
    {
        switch (Reason)
        {
        case DLL_PROCESS_ATTACH:
            On_Loader_Attach(Instance);
            break;
        case DLL_PROCESS_DETACH:
            Factory::On_Loader_Detach();
            break;
        default:
            break;
        }
        return true;
    }
}