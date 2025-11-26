#pragma once

#include "HelpPage.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct HelpPage : HelpPageT<HelpPage>
    {
        HelpPage();
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct HelpPage : HelpPageT<HelpPage, implementation::HelpPage>
    {
    };
}
