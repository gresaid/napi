#include <napi.h>
#include <windows.h>
#include <lm.h>
#include <sstream>
#include <string>

#pragma comment(lib, "Netapi32.lib")

std::string Utf16ToUtf8(const std::wstring& utf16Str) {
    int utf8Length = WideCharToMultiByte(CP_UTF8, 0, utf16Str.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Length > 0) {
        std::string utf8Str(utf8Length, '\0');
        WideCharToMultiByte(CP_UTF8, 0, utf16Str.c_str(), -1, &utf8Str[0], utf8Length, NULL, NULL);
        return utf8Str;
    }
    return "";
}

Napi::String CheckPrivilege(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    DWORD dwLevel = 2;
    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    LPUSER_INFO_2 pBuf = NULL;
    LPUSER_INFO_2 pTmpBuf;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    NET_API_STATUS nStatus;

    nStatus = NetUserEnum(NULL, dwLevel, FILTER_NORMAL_ACCOUNT, (LPBYTE*)&pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries, NULL);

    if (nStatus == NERR_Success) {
        if ((pTmpBuf = pBuf) != NULL) {
            std::ostringstream resultStream;
            for (DWORD i = 0; i < dwEntriesRead; i++) {
                std::wstring currentUsername = pTmpBuf->usri2_name;

                resultStream << " " << Utf16ToUtf8(currentUsername);

                LPUSER_INFO_3 pUserInfo3;
                NET_API_STATUS nStatusUserInfo = NetUserGetInfo(NULL, pTmpBuf->usri2_name, 3, (LPBYTE*)&pUserInfo3);

                if (nStatusUserInfo == NERR_Success) {
                    DWORD userPrivilege = pUserInfo3->usri3_priv;
                    resultStream << ":" << userPrivilege;
                    NetApiBufferFree(pUserInfo3);
                } else {
                    resultStream << ":3"; // Если произошла ошибка, присваиваем значение 3
                }

                pTmpBuf++;
            }
            if (pBuf != NULL) {
                NetApiBufferFree(pBuf);
            }
            return Napi::String::New(env, resultStream.str());
        }
    }

    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }

    return Napi::String::New(env, "error");
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "checkPrivilege"), Napi::Function::New(env, CheckPrivilege));
    return exports;
}

NODE_API_MODULE(addon, Init)
