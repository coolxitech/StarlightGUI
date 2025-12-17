#pragma once

#include "UpdateDialog.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct UpdateDialog : UpdateDialogT<UpdateDialog>
    {
        UpdateDialog();

        bool IsUpdate() const { return m_isUpdate; }
		void IsUpdate(bool value) { m_isUpdate = value; }

        hstring LatestVersion() const { return m_latestVersion; }
        void LatestVersion(const hstring& value) { m_latestVersion = value; }

        hstring GetAnLine(int line) const {
            switch (line) {
            case 1:
                return m_an_line1;
            case 2:
                return m_an_line2;
            case 3:
                return m_an_line3;
            default:
                return L"";
            }
        }

        void SetAnLine(int line, const hstring& msg) {
            switch (line) {
            case 1:
                m_an_line1 = msg;
                break;
            case 2:
                m_an_line2 = msg;
                break;
            case 3:
                m_an_line3 = msg;
                break;
            default:
                break;
			}
        }

        void OnPrimaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender,
            winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args);

    private:
        bool m_isUpdate{ false };
        hstring m_latestVersion{ L"" };
        hstring m_an_line1{ L"" };
        hstring m_an_line2{ L"" };
        hstring m_an_line3{ L"" };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct UpdateDialog : UpdateDialogT<UpdateDialog, implementation::UpdateDialog>
    {
    };
}
