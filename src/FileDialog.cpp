// Copyright (C) 2021-2025 by Nikolay Vladimirskiy - kolya.vladimirsky@gmail.com
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

#if 0 // WIN32

#include "FileDialog.h"
#include <optional>
#include <shobjidl.h>
#include <sstream>
#include <windows.h>

// USE ONLY LATIN LETTER PATH NAMES
std::optional<std::string> FileDialog::openDir()
{
    std::optional<std::string> result;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                                      COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* p_file_open;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                              IID_IFileOpenDialog, reinterpret_cast<void**>(&p_file_open));

        if (SUCCEEDED(hr))
        {
            // Show the Open dialog box.
            hr = p_file_open->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem* p_item;
                hr = p_file_open->GetResult(&p_item);
                if (SUCCEEDED(hr))
                {
                    PWSTR psz_file_path;
                    hr = p_item->GetDisplayName(SIGDN_FILESYSPATH, &psz_file_path);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                        std::wstringstream ss;
                        ss << psz_file_path;
                        std::wstring wstr = ss.str();
                        result = std::string(wstr.begin(), wstr.end());
                        MessageBoxW(NULL, psz_file_path, L"File Path", MB_OK);
                        CoTaskMemFree(psz_file_path);
                    }
                    p_item->Release();
                }
            }
            p_file_open->Release();
        }
        CoUninitialize();
    }
    return result;
}

#endif
