#include "pch.h"
#include "FilePage.xaml.h"
#if __has_include("FilePage.g.cpp")
#include "FilePage.g.cpp"
#endif

#include <chrono>
#include <shellapi.h>
#include <CopyFileDialog.xaml.h>

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

	void FilePage::FileListView_RightTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e)
	{
        auto listView = FileListView();

        if (auto fe = e.OriginalSource().try_as<FrameworkElement>())
        {
            auto container = FindParent<ListViewItem>(fe);
            if (container)
            {
                if (listView.SelectedItems().Size() < 2) listView.SelectedItem(container.Content());
            }
        }

        if (!listView.SelectedItems() || listView.SelectedItems().Size() == 0 || 
            // 只选择"上个文件夹"时不显示，多选的话在下面跳过处理，认为是误选
            (listView.SelectedItems().Size() == 1 && listView.SelectedItems().GetAt(0).as<winrt::StarlightGUI::FileInfo>().Flag() == 999)) 
            return;

        auto list = listView.SelectedItems();
        
        std::vector<winrt::StarlightGUI::FileInfo> selectedFiles;

        for (const auto& file : list) {
            auto item = file.as<winrt::StarlightGUI::FileInfo>();
            // 跳过"上个文件夹"选项
            if (item.Flag() == 999) continue;
            selectedFiles.push_back(item);
        }

        MenuFlyout menuFlyout;

        /*
        * 注意，由于这里是磁盘IO，我们不要使用异步，否则刷新时可能会出问题
        */
        // 选项1.1
        MenuFlyoutItem item1_1;
        item1_1.Icon(CreateFontIcon(L"\ue8e5"));
        item1_1.Text(L"打开");
        item1_1.Click([this, selectedFiles](IInspectable const& sender, RoutedEventArgs const& e) {
            for (const auto& item : selectedFiles) {
                if (item.Directory()) {
                    currentDirectory = currentDirectory + L"\\" + item.Name();
                    LoadFileList();
                }
                else ShellExecuteW(nullptr, L"open", item.Path().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            }
            });
        // 当选中多个内容并且其中一个是文件夹时禁用打开按钮
        if (selectedFiles.size() > 1) {
            for (const auto& item : selectedFiles) {
                if (item.Directory()) {
                    item1_1.IsEnabled(false);
                    break;
                }
            }
        }

        MenuFlyoutSeparator separator1;

        // 选项2.1
        MenuFlyoutItem item2_1;
        item2_1.Icon(CreateFontIcon(L"\ue74d"));
        item2_1.Text(L"删除");
        item2_1.Click([this, selectedFiles](IInspectable const& sender, RoutedEventArgs const& e) {
            for (const auto& item : selectedFiles) {
                if (KernelInstance::DeleteFileAuto(item.Path().c_str())) {
                    CreateInfoBarAndDisplay(L"成功", L"成功删除文件/文件夹: " + item.Name() + L" (" + item.Path() + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                    WaitAndReloadAsync(1000);
                }
                else CreateInfoBarAndDisplay(L"失败", L"无法删除文件/文件夹: " + item.Name() + L" (" + item.Path() + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            }
            });

        MenuFlyoutItem item2_2;
        item2_2.Icon(CreateFontIcon(L"\ue733"));
        item2_2.Text(L"删除 (内核)");
        item2_2.Click([this, selectedFiles](IInspectable const& sender, RoutedEventArgs const& e) {
            for (const auto& item : selectedFiles) {
                if (KernelInstance::_DeleteFileAuto(item.Path().c_str())) {
                    CreateInfoBarAndDisplay(L"成功", L"成功删除文件/文件夹: " + item.Name() + L" (" + item.Path() + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                    WaitAndReloadAsync(1000);
                }
                else CreateInfoBarAndDisplay(L"失败", L"无法删除文件/文件夹: " + item.Name() + L" (" + item.Path() + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            }
            });
        if (!KernelInstance::IsRunningAsAdmin()) item2_2.IsEnabled(false);

        MenuFlyoutItem item2_3;
        item2_3.Icon(CreateFontIcon(L"\uf5ab"));
        item2_3.Text(L"强制删除");
        item2_3.Click([this, selectedFiles](IInspectable const& sender, RoutedEventArgs const& e) {
            for (const auto& item : selectedFiles) {
                if (KernelInstance::MurderFileAuto(item.Path().c_str())) {
                    CreateInfoBarAndDisplay(L"成功", L"成功强制删除文件/文件夹: " + item.Name() + L" (" + item.Path() + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                    WaitAndReloadAsync(1000);
                }
                else CreateInfoBarAndDisplay(L"失败", L"无法强制删除文件/文件夹: " + item.Name() + L" (" + item.Path() + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            }
            });
        if (!KernelInstance::IsRunningAsAdmin()) item2_3.IsEnabled(false);

        MenuFlyoutItem item2_4;
        item2_4.Icon(CreateFontIcon(L"\ue72e"));
        item2_4.Text(L"锁定");
        item2_4.Click([this, selectedFiles](IInspectable const& sender, RoutedEventArgs const& e) {
            for (const auto& item : selectedFiles) {
                if (KernelInstance::LockFile(item.Path().c_str())) {
                    CreateInfoBarAndDisplay(L"成功", L"成功锁定文件: " + item.Name() + L" (" + item.Path() + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                    WaitAndReloadAsync(1000);
                }
                else CreateInfoBarAndDisplay(L"失败", L"无法锁定文件: " + item.Name() + L" (" + item.Path() + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            }
            });
        if (!KernelInstance::IsRunningAsAdmin()) item2_4.IsEnabled(false);
        for (const auto& item : selectedFiles) {
            if (item.Directory()) {
                item2_4.IsEnabled(false);
                break;
            }
        }

        MenuFlyoutItem item2_5;
        item2_5.Icon(CreateFontIcon(L"\ue8c8"));
        item2_5.Text(L"复制");
        item2_5.Click([this, selectedFiles](IInspectable const& sender, RoutedEventArgs const& e) {
            CopyFiles();
            });
        if (!KernelInstance::IsRunningAsAdmin()) item2_5.IsEnabled(false);

        menuFlyout.Items().Append(item1_1);
        menuFlyout.Items().Append(separator1);
        menuFlyout.Items().Append(item2_1);
        menuFlyout.Items().Append(item2_2);
        menuFlyout.Items().Append(item2_3);
        menuFlyout.Items().Append(item2_4);
        menuFlyout.Items().Append(item2_5);

        menuFlyout.ShowAt(listView, e.GetPosition(listView));
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
        if (m_isLoadingFiles) co_return;
        m_isLoadingFiles = true;

        LoadingRing().IsActive(true);

        auto start = std::chrono::steady_clock::now();

        auto lifetime = get_strong();

        std::wstring path = FixBackSplash(currentDirectory);
        currentDirectory = path;
        PathBox().Text(currentDirectory);

        // 简单判断根目录
        PreviousButton().IsEnabled(path.length() > 3);
        ResetState();
        m_allFiles.clear();

        co_await winrt::resume_background();

        try {
            if (KernelInstance::IsRunningAsAdmin()) KernelInstance::QueryFile(path, m_allFiles);
            else QueryFile(path, m_allFiles);
        }
        catch (...) {}      


        for (const auto& file : m_allFiles) {
            co_await GetFileInfoAsync(file);

            file.Path(FixBackSplash(file.Path()));
        }

        co_await wil::resume_foreground(DispatcherQueue());

        ApplySort(currentSortingOption, currentSortingType);

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

        // 立刻加载一次
        LoadMoreFiles();

        m_isLoadingFiles = false;
    }

    winrt::Windows::Foundation::IAsyncAction FilePage::GetFileInfoAsync(const winrt::StarlightGUI::FileInfo& file)
    {
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile(file.Path().c_str(), &findFileData);

        if (hFind != INVALID_HANDLE_VALUE) {
            if (file.SizeULong() == 0) {
                file.SizeULong(((ULONG64)findFileData.nFileSizeHigh << 32) | findFileData.nFileSizeLow);
            }
            file.Size(FormatMemorySize(file.SizeULong()));

            file.ModifyTimeULong(((ULONG64)findFileData.ftLastAccessTime.dwHighDateTime << 32) | findFileData.ftLastAccessTime.dwLowDateTime);
            SYSTEMTIME st;
            if (FileTimeToSystemTime(&findFileData.ftLastAccessTime, &st))
            {
                std::wstringstream ss;
                ss << std::setw(4) << std::setfill(L'0') << st.wYear << L"/"
                    << std::setw(2) << std::setfill(L'0') << st.wMonth << L"/"
                    << std::setw(2) << std::setfill(L'0') << st.wDay << L" "
                    << std::setw(2) << std::setfill(L'0') << st.wHour << L":"
                    << std::setw(2) << std::setfill(L'0') << st.wMinute << L":"
                    << std::setw(2) << std::setfill(L'0') << st.wSecond;
                file.ModifyTime(ss.str());
            }
            else
            {
                file.ModifyTime(L"(未知)");
            }

            FindClose(hFind);
        }
        else {
            file.Size(L"-1 (未知)");
            file.ModifyTime(L"(未知)");
        }

        if (file.Directory()) file.Size(L"");

        co_return;
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

        WaitAndReloadAsync(200);
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

        if (columnName == L"Previous") {
            currentDirectory = GetParentDirectory(currentDirectory.c_str());
            LoadFileList();
            return;
        }

        if (columnName == L"Name")
        {
            ApplySort(m_isNameAscending, "Name");
        }
        else if (columnName == L"ModifyTime")
        {
            ApplySort(m_isModifyTimeAscending, "ModifyTime");
        }
        else if (columnName == L"Size")
        {
            ApplySort(m_isSizeAscending, "Size");
        }

        ResetState();
        AddPreviousItem();
    }

    // 排序切换
    winrt::fire_and_forget FilePage::ApplySort(bool& isAscending, const std::string& column)
    {
        NameHeaderButton().Content(box_value(L"文件"));
        ModifyTimeHeaderButton().Content(box_value(L"修改时间"));
        SizeHeaderButton().Content(box_value(L"大小"));

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
        } else if (column == "ModifyTime") {
            if (isAscending) {
                ModifyTimeHeaderButton().Content(box_value(L"修改时间 ↓"));
                std::sort(m_allFiles.begin(), m_allFiles.end(), [](auto a, auto b) {
                    return a.ModifyTimeULong() < b.ModifyTimeULong();
                    });

            }
            else {
                ModifyTimeHeaderButton().Content(box_value(L"修改时间 ↑"));
                std::sort(m_allFiles.begin(), m_allFiles.end(), [](auto a, auto b) {
                    return a.ModifyTimeULong() > b.ModifyTimeULong();
                    });
            }
        } else if (column == "Size") {
            if (isAscending) {
                SizeHeaderButton().Content(box_value(L"大小 ↓"));
                std::sort(m_allFiles.begin(), m_allFiles.end(), [](auto a, auto b) {
                    return a.SizeULong() < b.SizeULong();
                    });

            }
            else {
                SizeHeaderButton().Content(box_value(L"大小 ↑"));
                std::sort(m_allFiles.begin(), m_allFiles.end(), [](auto a, auto b) {
                    return a.SizeULong() > b.SizeULong();
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

    winrt::fire_and_forget FilePage::NextDriveButton_Click(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        static std::vector<std::wstring> drives;
        static int currentIndex = 1;

        if (drives.empty()) {
            wchar_t driveBuffer[256];
            GetLogicalDriveStrings(255, driveBuffer);

            for (wchar_t* drive = driveBuffer; *drive; drive += wcslen(drive) + 1) {
                drives.push_back(drive);
            }

            if (drives.empty()) drives = { L"C:\\" };
        }

        currentDirectory = drives[currentIndex];
        currentIndex = (currentIndex + 1) % drives.size();

        co_await LoadFileList();
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
        // 简单判断根目录
        if (currentDirectory.size() <= 3) return;
        if (m_fileList.Size() > 0 && m_fileList.GetAt(0).Flag() == 999) return;
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

    winrt::Windows::Foundation::IAsyncAction FilePage::WaitAndReloadAsync(int interval) {
        auto lifetime = get_strong();

        reloadTimer.Stop();
        reloadTimer.Interval(std::chrono::milliseconds(interval));
        reloadTimer.Tick([this](auto&&, auto&&) {
            LoadFileList();
            reloadTimer.Stop();
            });
        reloadTimer.Start();

        co_return;
    }

    template <typename T>
    T FilePage::FindParent(DependencyObject const& child)
    {
        DependencyObject parent = VisualTreeHelper::GetParent(child);
        while (parent && !parent.try_as<T>())
        {
            parent = VisualTreeHelper::GetParent(parent);
        }
        return parent.try_as<T>();
    }

    void FilePage::QueryFile(std::wstring path, std::vector<winrt::StarlightGUI::FileInfo>& files) {
        std::wstring searchPath = path + L"\\*";

        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            return;
        }

        do {
            if (findFileData.cFileName[0] == L'.') {
                continue;
            }

            auto fileInfo = winrt::make<winrt::StarlightGUI::implementation::FileInfo>();
            fileInfo.Name(findFileData.cFileName);
            fileInfo.Directory((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
            fileInfo.Flag((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ? MFT_RECORD_FLAG_FILE : MFT_RECORD_FLAG_DIRECTORY);
            fileInfo.Path(path + L"\\" + findFileData.cFileName);

            files.push_back(fileInfo);

        } while (FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
    }

    winrt::fire_and_forget FilePage::CopyFiles() {
        auto dialog = winrt::make<winrt::StarlightGUI::implementation::CopyFileDialog>();
        dialog.XamlRoot(this->XamlRoot());

        auto result = co_await dialog.ShowAsync();

        if (result == ContentDialogResult::Primary) {
            std::wstring copyPath = dialog.CopyPath().c_str();

            std::vector<winrt::StarlightGUI::FileInfo> selectedFiles;

            for (const auto& file : FileListView().SelectedItems()) {
                auto item = file.as<winrt::StarlightGUI::FileInfo>();
                // 跳过上个文件夹选项
                if (item.Flag() == 999) continue;
                selectedFiles.push_back(item);
            }

            for (const auto& item : selectedFiles) {
                if (KernelInstance::_CopyFile(std::wstring(item.Path().c_str()).substr(0, item.Path().size() - item.Name().size()), copyPath + L"\\" + item.Name().c_str(), item.Name().c_str())) {
                    CreateInfoBarAndDisplay(L"成功", L"成功复制文件至: " + dialog.CopyPath(), InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                    WaitAndReloadAsync(1000);
                }
                else CreateInfoBarAndDisplay(L"失败", L"无法复制文件, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            }
        }
    }
}