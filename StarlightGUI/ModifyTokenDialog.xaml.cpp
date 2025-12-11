#include "pch.h"
#include "ModifyTokenDialog.xaml.h"
#if __has_include("ModifyTokenDialog.g.cpp")
#include "ModifyTokenDialog.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::StarlightGUI::implementation
{
    ModifyTokenDialog::ModifyTokenDialog()
    {
        InitializeComponent();
    }

    void ModifyTokenDialog::OnPrimaryButtonClick(ContentDialog const& sender,
        ContentDialogButtonClickEventArgs const& args)
    {
        auto deferral = args.GetDeferral();

        m_token = TokenComboBox().SelectedIndex();

        deferral.Complete();
    }
}
