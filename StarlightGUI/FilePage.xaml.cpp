#include "pch.h"
#include "FilePage.xaml.h"
#if __has_include("FilePage.g.cpp")
#include "FilePage.g.cpp"
#endif

#include <chrono>
#include <shellapi.h>

using namespace winrt;
using namespace WinUI3Package;
using namespace Microsoft::UI::Text;
using namespace Microsoft::UI::Xaml;

namespace winrt::StarlightGUI::implementation
{
	hstring currentDirectory = L"C:\\";
    static std::unordered_map<hstring, std::optional<winrt::Microsoft::UI::Xaml::Media::ImageSource>> iconCache;
    static HDC hdc{ nullptr };
    static bool loaded;

	FilePage::FilePage() {
		InitializeComponent();

        loaded = false;

        hdc = GetDC(NULL);
		FileListView().ItemsSource(m_fileList);

        if (!KernelInstance::IsRunningAsAdmin()) {
            RefreshButton().IsEnabled(false);
            FileCountText().Text(L"请以管理员身份运行！");
            CreateInfoBarAndDisplay(L"警告", L"请以管理员身份运行！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
        }
        else {
            m_scrollCheckTimer = winrt::Microsoft::UI::Xaml::DispatcherTimer();
            m_scrollCheckTimer.Interval(std::chrono::milliseconds(100));
            m_scrollCheckTimer.Tick([this](auto&&, auto&&) {
                CheckAndLoadMoreItems();
                });

            this->Loaded([this](auto&&, auto&&) {
                m_scrollCheckTimer.Start();
                LoadFileList();
                loaded = true;
                });

            this->Unloaded([this](auto&&, auto&&) {
                if (m_scrollCheckTimer) {
                    m_scrollCheckTimer.Stop();
                }
                ReleaseDC(NULL, hdc);
                });
        }
	}

	void FilePage::FileListView_RightTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e)
	{
		// TODO
	}

	void FilePage::FileListView_DoubleTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs const& e)
	{
        if (!FileListView().SelectedItem()) return;

        auto item = FileListView().SelectedItem().as<winrt::StarlightGUI::FileInfo>();

        if (item.Flag() == 999) {
            currentDirectory = GetParentDirectory(currentDirectory.c_str());
            LoadFileList();
        } else if (item.Directory()) {
            currentDirectory = currentDirectory + L"\\" + item.Name();
            LoadFileList();
        }
        else {
            ShellExecuteW(nullptr, L"open", item.Path().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        }
	}

    void FilePage::CheckAndLoadMoreItems() {
        if (!m_listScrollViewer && !FindScrollViewer(FileListView())) return;
        if (m_isLoadingMore || !m_hasMoreFiles) return;
        LoadMoreFiles();
    }

    winrt::fire_and_forget FilePage::LoadMoreFiles() {
        if (m_isLoadingMore || m_loadedCount >= m_allFiles.size()) {
            m_hasMoreFiles = false;
            co_return;
        }
        m_isLoadingMore = true;

        auto lifetime = get_strong();

        try {
            size_t start = m_loadedCount;
            size_t end = (start + 50) < m_allFiles.size() ? (start + 50) : m_allFiles.size();

            co_await wil::resume_foreground(DispatcherQueue());

            winrt::hstring query = SearchBox().Text();

            for (size_t i = start; i < end; ++i) {
                bool shouldRemove = query.empty() ? false : ApplyFilter(m_allFiles[i], query);
                if (shouldRemove) continue;

                co_await GetFileIconAsync(m_allFiles[i]);
                m_fileList.Append(m_allFiles[i]);
            }

            m_loadedCount = end;
            m_hasMoreFiles = (m_loadedCount < m_allFiles.size());
        }
        catch (...) {
        }

        m_isLoadingMore = false;
    }

    winrt::Windows::Foundation::IAsyncAction FilePage::LoadFileList()
    {
        if (!KernelInstance::IsRunningAsAdmin()) {
            co_return;
        }

        LoadingRing().IsActive(true);

        auto start = std::chrono::steady_clock::now();

        auto lifetime = get_strong();

        std::wstring path = FixBackSplash(currentDirectory);
        currentDirectory = path;
        PathBox().Text(currentDirectory);

        ResetState();
        m_allFiles.clear();

        co_await winrt::resume_background();

        try {
            KernelInstance::QueryFile(path, m_allFiles);
        }
        catch (...) {}        

        co_await wil::resume_foreground(DispatcherQueue());

        ApplySort(currentSortingOption, currentSortingType);

        for (const auto& file : m_allFiles) {
            file.Path(FixBackSplash(file.Path()));
        }

        // 将文件夹放在文件前面
        std::stable_sort(m_allFiles.begin(), m_allFiles.end(), [](const auto& a, const auto& b) {
            if (a.Directory() && !b.Directory()) {
                return true;
            }
            return false;
            });


        AddPreviousItem();

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::wstringstream countText;
        countText << L"共 " << m_allFiles.size() << L" 个文件 (" << duration << " ms)";
        FileCountText().Text(countText.str());
        LoadingRing().IsActive(false);

        LoadMoreFiles();
    }

    winrt::Windows::Foundation::IAsyncAction FilePage::GetFileIconAsync(const winrt::StarlightGUI::FileInfo& file)
    {
        if (iconCache.find(file.Path()) == iconCache.end()) {
            SHFILEINFO shfi;
            if (!SHGetFileInfoW(file.Path().c_str(), 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON)) {
                SHGetFileInfoW(L"", 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON);
            }
            ICONINFO iconInfo;
            if (GetIconInfo(shfi.hIcon, &iconInfo)) {
                BITMAP bmp;
                GetObject(iconInfo.hbmColor, sizeof(bmp), &bmp);
                BITMAPINFOHEADER bmiHeader = { 0 };
                bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmiHeader.biWidth = bmp.bmWidth;
                bmiHeader.biHeight = bmp.bmHeight;
                bmiHeader.biPlanes = 1;
                bmiHeader.biBitCount = 32;
                bmiHeader.biCompression = BI_RGB;

                int dataSize = bmp.bmWidthBytes * bmp.bmHeight;
                std::vector<BYTE> buffer(dataSize);

                GetDIBits(hdc, iconInfo.hbmColor, 0, bmp.bmHeight, buffer.data(), reinterpret_cast<BITMAPINFO*>(&bmiHeader), DIB_RGB_COLORS);

                winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap writeableBitmap(bmp.bmWidth, bmp.bmHeight);

                // 将数据写入 WriteableBitmap
                uint8_t* data = writeableBitmap.PixelBuffer().data();
                int rowSize = bmp.bmWidth * 4;
                for (int i = 0; i < bmp.bmHeight; ++i) {
                    int srcOffset = i * rowSize;
                    int dstOffset = (bmp.bmHeight - 1 - i) * rowSize;
                    std::memcpy(data + dstOffset, buffer.data() + srcOffset, rowSize);
                }

                DeleteObject(iconInfo.hbmColor);
                DeleteObject(iconInfo.hbmMask);
                DestroyIcon(shfi.hIcon);

                // 将图标缓存到 map 中
                iconCache[file.Path()] = writeableBitmap.as<winrt::Microsoft::UI::Xaml::Media::ImageSource>();
            }
        }
        file.Icon(iconCache[file.Path()].value());

        co_return;
    }

    void FilePage::SearchBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) {
        if (!loaded) return;

        reloadTimer.Stop();
        reloadTimer.Interval(std::chrono::milliseconds(200));
        reloadTimer.Tick([this](auto&&, auto&&) {
            ResetState();
            AddPreviousItem();
            reloadTimer.Stop();
            });
        reloadTimer.Start();
    }

    void FilePage::PathBox_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e) {
        if (e.Key() == winrt::Windows::System::VirtualKey::Enter)
        {
            try
            {
                fs::path path(PathBox().Text().c_str());
                if (fs::exists(path) && fs::is_directory(path)) {
                    currentDirectory = PathBox().Text();
                }
            }
            catch (...) {}
            PathBox().Text(currentDirectory);
            LoadFileList();
            e.Handled(true);
        }
    }

    bool FilePage::ApplyFilter(const winrt::StarlightGUI::FileInfo& file, hstring& query) {
        std::wstring name = file.Name().c_str();
        std::wstring queryWStr = query.c_str();

        // 不比较大小写
        std::transform(name.begin(), name.end(), name.begin(), ::towlower);
        std::transform(queryWStr.begin(), queryWStr.end(), queryWStr.begin(), ::towlower);

        return name.find(queryWStr) == std::wstring::npos;
    }

    void FilePage::ColumnHeader_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        Button clickedButton = sender.as<Button>();
        winrt::hstring columnName = clickedButton.Tag().as<winrt::hstring>();

        if (columnName == L"Name")
        {
            ApplySort(m_isNameAscending, "Name");
        }

        ResetState();
        AddPreviousItem();
    }

    // 排序切换
    winrt::fire_and_forget FilePage::ApplySort(bool& isAscending, const std::string& column)
    {
        NameHeaderButton().Content(box_value(L"文件"));

        if (column == "Name") {
            if (isAscending) {
                NameHeaderButton().Content(box_value(L"文件 ↓"));
                std::sort(m_allFiles.begin(), m_allFiles.end(), [](auto a, auto b) {
                    std::wstring aName = a.Name().c_str();
                    std::wstring bName = b.Name().c_str();
                    std::transform(aName.begin(), aName.end(), aName.begin(), ::towlower);
                    std::transform(bName.begin(), bName.end(), bName.begin(), ::towlower);

                    return aName < bName;
                    });

            }
            else {
                NameHeaderButton().Content(box_value(L"文件 ↑"));
                std::sort(m_allFiles.begin(), m_allFiles.end(), [](auto a, auto b) {
                    std::wstring aName = a.Name().c_str();
                    std::wstring bName = b.Name().c_str();
                    std::transform(aName.begin(), aName.end(), aName.begin(), ::towlower);
                    std::transform(bName.begin(), bName.end(), bName.begin(), ::towlower);

                    return aName > bName;
                    });
            }
        }

        isAscending = !isAscending;
        currentSortingOption = !isAscending;
        currentSortingType = column;

        co_return;
    }

    winrt::fire_and_forget FilePage::RefreshButton_Click(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        RefreshButton().IsEnabled(false);
        co_await LoadFileList();
        RefreshButton().IsEnabled(true);
        co_return;
    }

    bool FilePage::FindScrollViewer(DependencyObject parent) {
        if (!m_listScrollViewer) {
            if (auto sv = parent.try_as<winrt::Microsoft::UI::Xaml::Controls::ScrollViewer>()) {
                m_listScrollViewer = sv;
                return true;
            }
            auto childrenCount = winrt::Microsoft::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(parent);
            for (int i = 0; i < childrenCount; ++i) {
                auto child = winrt::Microsoft::UI::Xaml::Media::VisualTreeHelper::GetChild(parent, i);
                auto result = FindScrollViewer(child);
                if (result) break;
            }
            if (!m_listScrollViewer) return false;
        }
        return true;
    }

    void FilePage::AddPreviousItem() {
        auto previousPage = winrt::make<winrt::StarlightGUI::implementation::FileInfo>();
        previousPage.Name(L"上个文件夹");
        previousPage.Flag(999);
        GetFileIconAsync(previousPage);
        m_fileList.InsertAt(0, previousPage);
    }

    void FilePage::ResetState() {
        m_fileList.Clear();
        m_loadedCount = 0;
        m_isLoadingMore = false;
        m_hasMoreFiles = true;
    }
}