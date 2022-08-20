// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021 by Agustin Alvarez. All rights reserved.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Factory-Menu.hpp"
#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/combox.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   DATA   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Elixir::Factory::Menu
{
    std::thread                    s_Thread;
    std::shared_ptr<nana::form>    s_Form;
    std::shared_ptr<nana::label>   s_Ready_2;
    std::shared_ptr<nana::label>   s_Energy_2;
    std::shared_ptr<nana::listbox> s_Plays_1;
    std::shared_ptr<nana::listbox> s_Plays_2;
    std::shared_ptr<nana::combox>  s_Draws_1;
    std::shared_ptr<nana::combox>  s_Draws_2;
    std::shared_ptr<nana::combox>  s_Hand_1;
    std::shared_ptr<nana::combox>  s_Hand_2;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Elixir::Factory::Menu
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Create()
    {
#ifdef _PRO_VERSION
        constexpr auto WINDOW_TITLE  = L"Axie Oracle PRO  v1.0 (By \xD83D\xDC3A)";
        constexpr auto WINDOW_WIDTH  = 235;
        constexpr auto WINDOW_HEIGHT = 450;
#else
        constexpr auto WINDOW_TITLE  = L"Axie Oracle DEMO";
        constexpr auto WINDOW_WIDTH  = 125;
        constexpr auto WINDOW_HEIGHT = 65;
#endif

        s_Form = std::make_shared<nana::form>(
            nana::API::make_center(WINDOW_WIDTH, WINDOW_HEIGHT),
            nana::appearance(true, false, true, false, false, false, false));
        s_Form->caption(WINDOW_TITLE);
        s_Form->bgcolor(nana::color(255, 255, 255));

        s_Plays_1 = std::make_shared<nana::listbox>(* s_Form);
        s_Plays_1->column_resizable(false);
        s_Plays_1->column_movable(false);
        s_Plays_1->sortable(false);
        s_Plays_1->append_header(L"\xD83D\xDC7E", 105);
        s_Plays_1->append_header(L"\x26A1",  25);
        s_Plays_1->append_header(L"\x2694",  30);
        s_Plays_1->append_header(L"\x2665",  30);

        s_Draws_1 = std::make_shared<nana::combox>(* s_Form);
        s_Draws_1->editable(false);

        s_Hand_1 = std::make_shared<nana::combox>(* s_Form);
        s_Hand_1->editable(false);

        s_Ready_2 = std::make_shared<nana::label>(* s_Form, "<bold red size=12>Ready</>");
        s_Ready_2->format(true);

        s_Energy_2 = std::make_shared<nana::label>(* s_Form, "<size=12>Energy: 0 (0)</>");
        s_Energy_2->format(true);
        s_Plays_2 = std::make_shared<nana::listbox>(* s_Form);
        s_Plays_2->column_resizable(false);
        s_Plays_2->column_movable(false);
        s_Plays_2->sortable(false);
        s_Plays_2->append_header(L"\xD83D\xDC7E", 105);
        s_Plays_2->append_header(L"\x26A1",  25);
        s_Plays_2->append_header(L"\x2694",  30);
        s_Plays_2->append_header(L"\x2665",  30);

        s_Draws_2 = std::make_shared<nana::combox>(* s_Form);
        s_Draws_2->editable(false);

        s_Hand_2 = std::make_shared<nana::combox>(* s_Form);
        s_Hand_2->editable(false);

#ifdef _PRO_VERSION
        s_Form->div(R"(<vert margin=8 <vert fit HEADER> <weight=5%> <vert <weight=38% PLAY_1> <weight=5% HAND_1> <weight=5% DRAW_1> <weight=4%> <weight=38% PLAY_2> <weight=5% HAND_2> <weight=5% DRAW_2> > >)");

        (* s_Form)["HEADER"] << * s_Ready_2 << * s_Energy_2;
        (* s_Form)["PLAY_1"] << * s_Plays_1;
        (* s_Form)["HAND_1"] << * s_Hand_1;
        (* s_Form)["DRAW_1"] << * s_Draws_1;
        (* s_Form)["PLAY_2"] << * s_Plays_2;
        (* s_Form)["HAND_2"] << * s_Hand_2;
        (* s_Form)["DRAW_2"] << * s_Draws_2;
#else
        s_Form->div(R"(<vert margin=8 <vert fit HEADER>>)");

        (* s_Form)["HEADER"] << * s_Ready_2 << * s_Energy_2;
#endif // _PRO_VERSION

        s_Form->collocate();
        s_Form->show();

        // Execute
        s_Thread = std::thread([]() { nana::exec(); });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Delete()
    {
        s_Thread.~thread();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Set_Player(int32_t Energy, int32_t Delta, bool Ready)
    {
        wchar_t Caption_Ready [128];
        wchar_t Caption_Energy[128];

        wsprintfW(Caption_Ready, L"<bold %s size=12>Ready</>", Ready ? L"green" : L"red");

        if (Delta < 0)
        {
            wsprintfW(Caption_Energy, L"<size=12>Energy: %d (<red>%d</>)</>", Energy, -Delta);
        }
        else
        {
            wsprintfW(Caption_Energy, L"<size=12>Energy: %d (<green>%d</>)</>", Energy, Delta);
        }

         s_Ready_2->caption(Caption_Ready);
        s_Energy_2->caption(Caption_Energy);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Set_Active(int32_t Team, Controller::Cards & Cards)
    {
        std::shared_ptr<nana::listbox> Widget = (Team ? s_Plays_2 : s_Plays_1);
        Widget->clear();

        auto Category = Widget->at(0);

        uint32_t Energy = 0, Attack = 0, Defense = 0;

        for (const auto & Card : Cards)
        {
            Energy  += Card.Energy;
            Attack  += Card.Attack;
            Defense += Card.Defense;

            Category.append({
                Card.Name,
                std::to_wstring(Card.Energy),
                std::to_wstring(Card.Attack),
                std::to_wstring(Card.Defense)
            });
        }

        Category.append({ "TOTAL", std::to_string(Energy), std::to_string(Attack), std::to_string(Defense) });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Set_Hands(int32_t Team, Controller::Cards & Cards)
    {
        std::shared_ptr<nana::combox> Widget = (Team ? s_Hand_2 : s_Hand_1);
        Widget->clear();

        for (const Controller::Card & Card : Cards)
        {
            Widget->push_back(std::string( Card.Name.begin(), Card.Name.end() ));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Set_Secret(int32_t Team, Controller::Cards & Cards)
    {
        std::shared_ptr<nana::combox> Widget = (Team ? s_Draws_2 : s_Draws_1);
        Widget->clear();

        for (const Controller::Card & Card : Cards)
        {
            Widget->push_back(std::string( Card.Name.begin(), Card.Name.end() ));
        }
    }
}