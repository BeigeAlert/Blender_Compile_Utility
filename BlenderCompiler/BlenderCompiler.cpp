// BlenderCompiler.cpp : Defines the entry point for the console application.
//

#include <Windows.h>
#include <string>
#include <ShObjIdl.h>
#include <fstream>
#include <iostream>
#include <direct.h>

const std::wstring DIRECTORY_FILE_NAME = L"blender_directory.txt";
const std::wstring SCRIPT_FILE_NAME = L"blender_compile.py";
const std::wstring SCRIPTS_DIRECTORY = L"utils\\blender_scripts\\";

enum fileType
{
    ft_exe = 1
};

bool WriteToFile(std::wstring fileName, std::wstring& contents)
{
    std::wofstream outfile(fileName);
    if (!outfile.is_open())
    {
        return false;
    }
    outfile << contents.c_str();
    outfile.close();
    return true;
}

bool ReadFromFile(std::wstring fileName, std::wstring& contents)
{
    std::wifstream infile(fileName);
    if (!infile.is_open())
    {
        return false;
    }
    std::getline(infile, contents);  //only care about the one line
    infile.close();
    return true;
}

bool DoesFileExist(std::wstring fileName)
{
    std::ifstream infile(fileName);
    if (infile.good())
    {
        return true;
    }
    return false;
}

bool PromptFileLocation(std::wstring& resultPath, std::wstring title=L"Open", fileType filterType=ft_exe)
{
    HRESULT hr;
    
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (!SUCCEEDED(hr))
    {
        CoUninitialize();
        return false;
    }

    IFileOpenDialog *pFileOpen;
    // Create the FileOpenDialog object.
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
    if (!SUCCEEDED(hr))
    {
        pFileOpen->Release();
        CoUninitialize();
        return false;
    }

    // Set the filter type
    switch (filterType)
    {
    case ft_exe:
            {
                COMDLG_FILTERSPEC filter;
                filter.pszName = L"Windows Executable";
                filter.pszSpec = L"*.exe";
                hr = pFileOpen->SetFileTypes(1, &filter);
                break;
            }
    default:
        {
            break;
        }
    }
    if (!SUCCEEDED(hr))
    {
        pFileOpen->Release();
        CoUninitialize();
        return false;
    }

    // Set the title
    hr = pFileOpen->SetTitle(L"Please locate your Blender executable.");
    if (!SUCCEEDED(hr))
    {
        pFileOpen->Release();
        CoUninitialize();
        return false;
    }

    // Show the Open dialog box.
    hr = pFileOpen->Show(NULL);
    if (!SUCCEEDED(hr))
    {
        pFileOpen->Release();
        CoUninitialize();
        return false;
    }

    // Get the file name from the open dialog box.
    IShellItem *pItem;
    hr = pFileOpen->GetResult(&pItem);
    if (!SUCCEEDED(hr))
    {
        pItem->Release();
        pFileOpen->Release();
        CoUninitialize();
        return false;
    }

    PWSTR pszFilePath;
    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

    if (!SUCCEEDED(hr))
    {
        pItem->Release();
        pFileOpen->Release();
        CoUninitialize();
        CoTaskMemFree(pszFilePath);
        return false;
    }

    resultPath = pszFilePath;
    CoTaskMemFree(pszFilePath);
    pItem->Release();
    pFileOpen->Release();
    CoUninitialize();
    return true;
}

bool PromptForBlender(std::wstring& blenderPath)
{
    if (!PromptFileLocation(blenderPath, L"Locate Blender.exe", ft_exe))
    {
        return false;
    }
    WriteToFile(DIRECTORY_FILE_NAME, blenderPath);
    return true;
}

bool EnsureValidBlenderDirectory(std::wstring& blenderPath)
{
    if (!DoesFileExist(DIRECTORY_FILE_NAME))
    {
        // No text file, period
        int result = MessageBox(NULL, L"Please locate your blender installation.", L"First time setup", MB_OKCANCEL | MB_ICONINFORMATION);
        if (result != IDOK)
        {
            return false;
        }
        return PromptForBlender(blenderPath);
    }
    else
    {
        // text file exists...
        ReadFromFile(DIRECTORY_FILE_NAME, blenderPath);

        if (!DoesFileExist(blenderPath))
        {
            // ...but doesn't point to valid blender executable...
            int result = MessageBox(NULL, L"Unable to locate blender.exe in the provided location.", L"Error", MB_OKCANCEL | MB_ICONERROR);
            if (result != IDOK)
            {
                return false;
            }
            return PromptForBlender(blenderPath);
        }
        return true;
    }
}

std::wstring sToW(const std::string& s)
{
    std::wstring w;
    w.assign(s.begin(), s.end());
    return w;
}

std::string getCurrentPath()
{
    char cCurrentPath[FILENAME_MAX];

    if (!_getcwd(cCurrentPath, sizeof(cCurrentPath)))
    {
        return "";
    }

    std::string path = cCurrentPath;
    if (path.back() != '\\')
    {
        path = path + '\\';
    }

    return path;
}

int main(int argc, char* argv[])
{
    std::wstring blenderPath;

    if (argc == 1)
    {
        // no arguments were provided -- user probably just tried to run it
        // we'll provide the opportunity to change the directory through this,
        // even if it was already valid.
        if (!DoesFileExist(DIRECTORY_FILE_NAME))
        {
            int result = MessageBox(NULL, L"It looks like Blender hasn't been located yet by this program.  Would you like to do this now?",
                L"First time setup", MB_YESNOCANCEL | MB_ICONINFORMATION);
            if (result == IDYES)
            {
                PromptForBlender(blenderPath);
            }
            return 0;
        }

        ReadFromFile(DIRECTORY_FILE_NAME, blenderPath);

        if (!DoesFileExist(blenderPath))
        {
            int result = MessageBox(NULL, L"Valid blender install not found.  Would you like to do this now?",
                L"First time setup", MB_YESNOCANCEL | MB_ICONINFORMATION);
            if (result == IDYES)
            {
                PromptForBlender(blenderPath);
            }
            return 0;
        }

        int result = MessageBox(NULL, L"Looks like a blender install has already been located.  Do you wish to change it?",
            L"Change blender location", MB_YESNOCANCEL | MB_ICONINFORMATION);
        if (result == IDYES)
        {
            PromptForBlender(blenderPath);
        }
        return 0;
    }
    
    if (!EnsureValidBlenderDirectory(blenderPath))
    {
        std::cerr << "Blender directory invalid!  Cannot proceed." << std::endl;
        return 1;
    }

    if (argc > 2)
    {
        std::cerr << "Too many arguments!" << std::endl;
        return 1;
    }

    // ensure pyton intermediate script exists.  Should be in same directory as this executable.
    if (!DoesFileExist(SCRIPT_FILE_NAME))
    {
        std::cerr << "Missing python script 'blender_compile.py'.  Please re-install the application." << std::endl;
        return 1;
    }

    // first argument is of course the execution directory of this file.
    // second argument should be the location of the .blend file they wish to compile.

    std::wstring blendFile = sToW(argv[1]);
    std::wstring command;
    std::wstring programDir = sToW(getCurrentPath());

    command = L"\"\"" + blenderPath + L"\" -b \"" + blendFile + L"\" -P " + SCRIPT_FILE_NAME + L" -- \"" + programDir + SCRIPTS_DIRECTORY + L"\"";

    _wsystem(command.c_str());

    return 0;
}

