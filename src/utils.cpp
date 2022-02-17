#include <fstream>
#include <sstream>
#include <memory>
#include <io.h>
#include <cstring>
#include <wtypes.h>

#include "utils.hpp"


std::string File::readText(const std::string& filename)
{

	std::ifstream file{ filename };
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filename);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::vector<char> File::readBinary(const std::string& filename)
{
	std::ifstream file{ filename, std::ios::binary | std::ios::ate };
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filename);
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	file.read(buffer.data(), size);
	return buffer;
}

std::vector<char*> File::readAllFilesInDir(const std::string& path)
{
    long long hFile = 0;
    //文件信息
    struct _finddata_t fileinfo;
    std::string p;
    std::vector<char*> files;

    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            if (!(fileinfo.attrib & _A_SUBDIR))
            {
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                {
                    std::string tmp = fileinfo.name;
                    tmp = tmp.substr(0, tmp.find_last_of('.'));
                    char* tmpChar = new char[128];
                    strcpy(tmpChar, tmp.c_str());
                    files.push_back(tmpChar);
                }
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
    
    return files;
}

std::vector<char*> File::readAllDirsInDir(const std::string& path) 
{
    long long hFile = 0;
    //文件信息
    struct _finddata_t fileinfo;
    std::string p;
    std::vector<char*> files;

    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            if ((fileinfo.attrib & _A_SUBDIR))
            {
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                {
                    char* tmpChar = new char[128];
                    strcpy(tmpChar, fileinfo.name);
                    files.push_back(tmpChar);
                }
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }

    return files;
}

std::vector<char*> File::readAllFilesInDirWithExt(const std::string& path)
{
    long long hFile = 0;
    //文件信息
    struct _finddata_t fileinfo;
    std::string p;
    std::vector<char*> files;

    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            if (!(fileinfo.attrib & _A_SUBDIR))
            {
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                {
                    char* tmpChar = new char[128];
                    strcpy(tmpChar, fileinfo.name);
                    files.push_back(tmpChar);
                }
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }

    return files;
}

std::string File::openFileDialog()
{
    TCHAR szBuffer[MAX_PATH] = { 0 };
    OPENFILENAME ofn;
    ZeroMemory(&ofn,sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetForegroundWindow();
    ofn.lpstrFilter = TEXT("All Files(*.*)\0*.*\0");//要选择的文件后缀  
    ofn.lpstrInitialDir = TEXT("./data/models");
    ofn.lpstrFile = szBuffer;//存放文件的缓冲区   
    ofn.nMaxFile = sizeof(szBuffer) / sizeof(*szBuffer);
    ofn.nFilterIndex = 0;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;//标志如果是多选要加上OFN_ALLOWMULTISELECT  
    BOOL bSel = GetOpenFileName(&ofn);
    if (bSel)
    {
        std::wstring tmp(szBuffer);
        return std::string(tmp.begin(), tmp.end());
    }
    else
        return "";
}
