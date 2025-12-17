#include "pch.h"
#include "UpdateDialog.xaml.h"
#if __has_include("UpdateDialog.g.cpp")
#include "UpdateDialog.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::StarlightGUI::implementation
{
    UpdateDialog::UpdateDialog() {
        InitializeComponent();

        this->Loaded([this](auto&&, auto&&) {
            if (IsUpdate()) {
                Title(winrt::box_value(L"发现更新"));
                LatestVersionText().Text(LatestVersion());
                PrimaryButtonText(L"下载");
                SecondaryButtonText(L"取消");
				UpdateStackPanel().Visibility(Visibility::Visible);
				AnnouncementStackPanel().Visibility(Visibility::Collapsed);
            }
            else {
                Title(winrt::box_value(L"公告"));
                UpdateTimeText().Text(LatestVersion());
                AnnouncementLine1().Text(GetAnLine(1));
                AnnouncementLine2().Text(GetAnLine(2));
                AnnouncementLine3().Text(GetAnLine(3));
                PrimaryButtonText(L"确认");
                UpdateStackPanel().Visibility(Visibility::Collapsed);
                AnnouncementStackPanel().Visibility(Visibility::Visible);
            }
            });
    }

    void UpdateDialog::OnPrimaryButtonClick(ContentDialog const& sender,
        ContentDialogButtonClickEventArgs const& args)
    {
        auto deferral = args.GetDeferral();

        if (!IsUpdate() && DontShowAgainCheckBox().IsChecked().GetBoolean()) {
			LOG_INFO(L"", L"Opted to not show announcements again today.");
            SaveConfig("last_announcement_date", GetDateAsInt());
        }

        deferral.Complete();
    }
}
