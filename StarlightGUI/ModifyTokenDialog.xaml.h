#pragma once

#include "ModifyTokenDialog.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct ModifyTokenDialog : ModifyTokenDialogT<ModifyTokenDialog>
    {
        ModifyTokenDialog();
            
        int Token() const { return m_token; }

        void OnPrimaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender,
            winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args);

    private:
        int m_token{ 0 };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct ModifyTokenDialog : ModifyTokenDialogT<ModifyTokenDialog, implementation::ModifyTokenDialog>
    {
    };
}
