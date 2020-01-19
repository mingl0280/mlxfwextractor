#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <regex>

#ifndef _WIN32
#define memcpy_s(_T1,_T2,_T3,_T4) memcpy(_T1,_T3,_T4)
#endif

using namespace std;
namespace fs = filesystem;

typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long ll;
typedef unsigned long long ull;
typedef unsigned char uchar;

struct MlnxInfoStruct
{
    char identifier[32]{0};
    string PN;
    string description;
};

map<uint, MlnxInfoStruct> FirmwareInfos;

void GetAllFwInfos();
void SplitAllFw();
vector<string> SplitString(string src, const string& delimiters);
void WriteFirmwareFolder(const uchar* file_cache, ll begin_pos, ll end_pos, int n_fw_index);


int main()
{
    GetAllFwInfos();
    SplitAllFw();
    return 0;
}


void GetAllFwInfos()
{
    ifstream fileIn("fwSect1", ifstream::in | ifstream::binary);
    if (!fileIn)
        return;
    int nFwIndex = 0;
    while(fileIn.peek() != EOF)
    {
        struct MlnxInfoStruct fw_info;
        char chStream[228] = {};
        fileIn.read(chStream, 228);
        memcpy_s(&fw_info.identifier, 32, chStream, 32);
        bool beginDataSect = false;
        bool pnWrite = false;
        for(int i=36;i<228;i++)
        {
            if (chStream[i - 2] == 'P' && chStream[i - 1] == 'N' && chStream[i] == 0)
            {
                beginDataSect = true;
                continue;
            }
            if (beginDataSect == false && chStream[i - 2] == 'O' && chStream[i - 1] == 'N' && chStream[i] == 0)
            {
                beginDataSect = true;
                continue;
            }
            if (!pnWrite && beginDataSect)
            {
                if (chStream[i] == 0)
                {
                    beginDataSect = false;
                    pnWrite = true;
                    continue;
                }
                fw_info.PN += chStream[i];
            }
            if (pnWrite && beginDataSect)
            {
                if (chStream[i] == 0)
                    break;
                fw_info.description += chStream[i];
            }
        }
        FirmwareInfos.insert(make_pair(nFwIndex, fw_info));
        nFwIndex++;
    }
    
}

const uchar kSigTemplate[8] = { 0x4D, 0x54, 0x46, 0x57,
                               0x8C, 0xDF, 0xD0, 0x00 };

void SplitAllFw()
{
    ifstream fileIn("fwSect3", ifstream::binary);
    if (!fileIn)
        return;
    auto n_fw_index = 0;
    const auto begin = fileIn.tellg();
    
    fileIn.seekg(0,ios::end);
    const streampos end = fileIn.tellg();
    const ll file_len = end - begin;
    fileIn.seekg(0,ios::beg);
    auto fileCache = new uchar[file_len];
    fileIn.read(reinterpret_cast<char*>(fileCache), file_len);
    ull begin_pos = 0;
    ull end_pos;
    for (ll index = 0; index < file_len; index++)
    {
        if (index > 8 && index < file_len - 8)
        {
            if (fileCache[index] == 0x4D && fileCache[index + 1] == 0x54)
            {
                uchar sig[8]{ 0 };
                memcpy_s(&sig, 8, fileCache + index, 8);
                bool isSame = true;
                for(int i=0;i<8;i++)
                {
                    if (sig[i] != kSigTemplate[i])
                    {
                        isSame = false;
                        break;
                    }
                }
                if (isSame)
                {
                    cout << "Firmware file #" << n_fw_index << " complete, writing to bin file...";
                    end_pos = index - 1;
                    WriteFirmwareFolder(fileCache, begin_pos, end_pos, n_fw_index);
                    begin_pos = index;
                    n_fw_index++;
                    cout << FirmwareInfos[n_fw_index].PN << "complete. " << endl;
                }
            }
        }
    }
    end_pos = file_len;
    WriteFirmwareFolder(fileCache, begin_pos, end_pos, n_fw_index);
    delete[] fileCache;
}

void WriteFirmwareFolder(const uchar* file_cache, ll begin_pos, ll end_pos, int n_fw_index)
{
    auto fw_len = end_pos - begin_pos + 1;
    char* fw_file_bytes = new char[fw_len] {0};
    memcpy_s(fw_file_bytes, fw_len, file_cache + begin_pos, fw_len);
    auto splitedDesc = SplitString(FirmwareInfos[n_fw_index].description, " ");
    fs::path newFolder(fs::current_path().string() + "\\Firmwares\\" + FirmwareInfos[n_fw_index].identifier + "." + FirmwareInfos[n_fw_index].PN);
    try
    {
        fs::create_directories(newFolder);
    }
    catch (exception&)
    {
        // ignore error
    }
    if (fs::exists(newFolder))
    {
        ofstream descFileWriter(newFolder.string() + "\\desc.txt");
        ofstream fwFileWriter(newFolder.string() + "\\" + FirmwareInfos[n_fw_index].PN + ".bin", ios::out | ios::trunc | ios::binary);
        descFileWriter << "Firmware ID:" << FirmwareInfos[n_fw_index].identifier << endl;
        descFileWriter << "Product Name:" << FirmwareInfos[n_fw_index].PN << endl;
        descFileWriter << "Description:" << FirmwareInfos[n_fw_index].description << endl;
        descFileWriter.flush();
        descFileWriter.close();
        fwFileWriter.write(fw_file_bytes, fw_len);
        fwFileWriter.flush();
        fwFileWriter.close();
    }
    delete[] fw_file_bytes;
}

vector<string> SplitString(string src, const string& delimiters)
{
    vector<string> retVal;
    string newDelimiters;
    for (char delimChar : delimiters)
    {
        if (delimChar == '\\')
            newDelimiters += "\\\\";
        else
            newDelimiters += delimChar;
    }
    regex words_regex("[^" + newDelimiters + "]+");
    auto words_begin = sregex_iterator(src.begin(), src.end(), words_regex);
    auto words_end = sregex_iterator();

    for (auto regexIterator = words_begin; regexIterator != words_end; ++regexIterator)
    {
        retVal.push_back((*regexIterator).str());
    }

    return retVal;
}
