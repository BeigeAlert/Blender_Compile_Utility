// BlenderCompiler.cpp : Defines the entry point for the console application.
//

#include <Windows.h>
#include <string>
#include <ShObjIdl.h>
#include <fstream>
#include <iostream>
#include <direct.h>
#include <vector>
#include <ctime>
#include <algorithm>
#include "BuilderXmlEntries.h"

// auto-generated header with the up-to-date python files literally embedded.
// run python_to_header.py to generate this file from the python scripts in the git directory
#include "generated_python_literals.h"

const std::wstring SCRIPT_FILE_NAME = L"blender_compile.py";

const std::wstring DIRECTORY_FILE_NAME = L"blender_directory.txt";
const std::wstring SCRIPTS_DIRECTORY = L"utils\\blender_scripts\\";

void DebugPopup(std::wstring message)
{
    MessageBox(NULL, message.c_str(), L"Debug", MB_OK | MB_ICONHAND);
}

enum fileType
{
    ft_exe = 1
};

std::wstring sToW(const std::string& s)
{
    std::wstring w;
    w.assign(s.begin(), s.end());
    return w;
}

std::string wToS(const std::wstring& w)
{
    std::string s;
    s.assign(w.begin(), w.end());
    return s;
}

bool VerifyScriptsDirectory()
{
    int result = _wmkdir(SCRIPTS_DIRECTORY.c_str());
    if (result != 0)
    {
        if (errno == EEXIST)
        {
            return true; //directory already existed
        }
        else if (errno == ENOENT)
        {
            return false; //directory could not be found
        }
    }
    return true; //directory didn't exist, and was created
}

bool WriteLinesToFile(std::wstring fileName, std::vector<std::wstring>& contents)
{
    if (!VerifyScriptsDirectory())
        return false;

    std::wofstream outfile(fileName);
    if (!outfile.is_open())
    {
        return false;
    }
    for (unsigned int i = 0; i < contents.size(); ++i)
    {
        outfile << contents[i].c_str();
    }
    outfile.close();
    return true;
}

bool WriteLinesToFile(std::wstring fileName, const std::vector<const std::wstring>& contents)
{
    if (!VerifyScriptsDirectory())
        return false;

    std::wofstream outfile(fileName);
    if (!outfile.is_open())
    {
        return false;
    }
    for (unsigned int i = 0; i < contents.size(); ++i)
    {
        outfile << contents[i].c_str();
    }
    outfile.close();
    return true;
}

bool WriteToFile(std::wstring fileName, const std::wstring& contents)
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
        infile.close();
        return true;
    }
    infile.close();
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
            if (!DoesFileExist(DIRECTORY_FILE_NAME))
            {
                return false;
            }
            else
            {
                return ReadFromFile(DIRECTORY_FILE_NAME, blenderPath);
            }
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
            int result = MessageBox(NULL, L"Unable to locate blender.exe in the provided location.", L"Error", MB_OK | MB_ICONERROR);
            if (result != IDOK)
            {
                return false;
            }
            return PromptForBlender(blenderPath);
        }
        return true;
    }
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

unsigned long long GetLastUpdatedValue(int i)
{
    std::wstring contents;
    if (!ReadFromFile(SCRIPTS_DIRECTORY + PYTHON_FILE_NAMES[i], contents)) // gets us just the first line.
        return 0;

    unsigned long long value;
    try {
        value = std::stoll(contents.substr(2, std::string::npos));
    }
    catch (const std::invalid_argument&)
    {
        return 0;
    }
    return value;
}

bool ReAquireScript(int i)
{
    return WriteLinesToFile(SCRIPTS_DIRECTORY + PYTHON_FILE_NAMES[i], PYTHON_FILE_DATAS[i]);
}

bool VerifyScript(int i)
{
    std::wstring file_name = SCRIPTS_DIRECTORY + PYTHON_FILE_NAMES[i];
    if (!DoesFileExist(file_name))
    {
        // file doesn't exist, copy the file over
        return ReAquireScript(i);
    }
    else
    {
        unsigned long long verify_last = GetLastUpdatedValue(i);
        if (verify_last == 0)
            return false;

        if (LAST_UPDATED != verify_last)
        {
            std::cerr << "Reaquiring script " << wToS(PYTHON_FILE_NAMES[i]) << std::endl;
            // file exists, but is out of date.  Overwrite it.
            return ReAquireScript(i);
        }
        return true;
    }
}

bool VerifyScripts()
{
    for (unsigned int i = 0; i < PYTHON_FILE_NAMES.size(); ++i)
    {
        if (!VerifyScript(i))
        {
            return false;
        }
    }
    return true;
}

bool ReadXmlFile(const std::wstring& XmlFilename, std::vector<std::wstring>& output)
{
    std::wifstream file(XmlFilename);
    std::wstring str;
    while (std::getline(file, str))
    {
        output.push_back(str + L"\n");
    }
    return true;
}

bool FindXmlFirstLine(const std::vector<std::wstring>& xmlFile, const std::wstring& source, int& lineNumber)
{
    // find the first line of the bundled xml entries in the xml file.
    for (unsigned int i = 0; i < xmlFile.size(); ++i)
    {
        const std::wstring& currentLine = xmlFile[i];
        size_t result = currentLine.find(source);
        if (result == std::wstring::npos) //not found
        {
            continue;
        }
        else
        {
            lineNumber = i;
            return true;
        }
    }
    return false;
}

bool CheckXmlMatch(const std::vector<std::wstring>& xmlFile)
{
    int lineNumber;
    FindXmlFirstLine(xmlFile, XmlEntries[0], lineNumber);

    if (XmlEntries.size() == 1)
    {
        return true;
    }

    for (unsigned int i = 1; i < XmlEntries.size(); ++i)
    {
        size_t result = xmlFile[lineNumber+i].find(XmlEntries[i]);
        if (result == std::wstring::npos)
        {
            return false;
        }
    }

    return true;  //made it this far, everything appears to be in order.
}

bool InsertXmlEntries(std::vector<std::wstring>& XmlContents)
{
    int lineNumber;
    std::wstring source = L"<rules>";
    if (!FindXmlFirstLine(XmlContents, source, lineNumber))
    {
        return false;
    }

    // figure out how much to indent.  Luckily, it's all tabs, so we can just count the characters.
    int numTabs = 0;
    size_t result = XmlContents[lineNumber].find(source);
    if (result != std::wstring::npos && result > 0)
    {
        numTabs = result + 1;
    }
    std::wstring tabs = L"";
    tabs.resize(numTabs, L'\t');

    ++lineNumber; //insert AFTER <rules>

    XmlContents.insert(XmlContents.begin() + lineNumber, tabs + L"\n");
    ++lineNumber;

    for (unsigned int i = 0; i < XmlEntries.size(); ++i)
    {
        XmlContents.insert(XmlContents.begin() + lineNumber, tabs + XmlEntries[i]);
        ++lineNumber;
    }

    return true;
}

bool RepairXmlFile(std::vector<std::wstring>& XmlContents)
{
    // Really, the only way we can repair the file is if the starting end ending comments are left intact,
    // so we'll start by trying to locate them.
    int firstLineNumber;
    if (!FindXmlFirstLine(XmlContents, XmlEntries.front(), firstLineNumber))
    {
        return false;
    }

    int lastLineNumber;
    if (!FindXmlFirstLine(XmlContents, XmlEntries.back(), lastLineNumber))
    {
        return false;
    }

    // Verify line numbers are okay, and remove.
    if (firstLineNumber >= lastLineNumber)
    {
        return false;
    }
    XmlContents.erase(XmlContents.begin() + firstLineNumber, XmlContents.begin() + lastLineNumber + 1);

    // Add lines back in.
    if (!InsertXmlEntries(XmlContents))
    {
        return false;
    }
    else
    {
        return WriteLinesToFile(XmlFileName, XmlContents);
    }
}

bool VerifyXmlEntries()
{
    if (!DoesFileExist(XmlFileName))
    {
        return false;
    }

    std::vector<std::wstring> XmlContents;
    ReadXmlFile(XmlFileName, XmlContents);

    int lineNumber;
    if (!FindXmlFirstLine(XmlContents, XmlEntries[0], lineNumber))
    {
        // Couldn't find the first line of the xml entries in the xml file, so we need to add them.
        if (!InsertXmlEntries(XmlContents))
        {
            return false;
        }
        else
        {
            return WriteLinesToFile(XmlFileName, XmlContents);
        }
    }
    else
    {
        // we found the first line, good enough.  Don't want to FORCE people to
        // setup this file exactly like me, but just want to add the entries if
        // they haven't got them already.
        
        // we'll check and see if what's there matches the internals of what we
        // have here.  If they match, we'll exit normally, otherwise we'll prompt
        // the user, asking them if they want us to repair the builder_setup.xml.

        if (CheckXmlMatch(XmlContents))
        {
            return true;  //matches, nothing to do here.
        }
        else
        {
            std::wstring message = L"It appears that the portion of builder_setup.xml that deals with "
                                   L"blender files is out of date, or has been altered.  Would you like "
                                   L"to repair this now?";
            int result = MessageBox(NULL, message.c_str(), L"Repair builder_setup.xml?", MB_YESNO | MB_ICONEXCLAMATION);
            if (result != IDYES)
            {
                return true;  // we tried, but they don't want it.
            }

            // attempt to repair XML file.
            if (!RepairXmlFile(XmlContents))
            {
                std::wstring message = L"Unable to repair xml file.  Delete builder_setup.xml, and run verify files through steam to replace it.  Then run this utility again.";
                MessageBox(NULL, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
            }
            return true;
        }
    }
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
        
        if (!VerifyXmlEntries())
        {
            std::wstring message = L"There was a problem verifying that " + XmlFileName + L" contains the correct "
                L"entries to make Builder work with this utility.";
            MessageBox(NULL, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
        }

        if (!VerifyScripts())
        {
            MessageBox(NULL, L"One or more scripts are missing and replacing them failed.", L"Error", MB_OK | MB_ICONERROR);
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

    if (!VerifyScripts())
    {
        std::cerr << "One or more scripts are missing and replacing them failed." << std::endl;
        return 1;
    }

    // first argument is of course the execution directory of this file.
    // second argument should be the location of the .blend file they wish to compile.

    std::wstring blendFile = sToW(argv[1]);
    std::wstring command;
    std::wstring programDir = sToW(getCurrentPath());

    command = L"\"\"" + blenderPath + L"\" -b \"" + blendFile + L"\" -P " + SCRIPTS_DIRECTORY + SCRIPT_FILE_NAME + L" -- \"" + programDir + SCRIPTS_DIRECTORY + L"\"";

    _wsystem(command.c_str());

    return 0;
}

