/*
Utils.cpp
Copyright (C) 2015  Belledonne Communications, Grenoble, France
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "Address.h"
#include "AuthInfo.h"
#include "Call.h"
#include "CallLog.h"
#include "CallParams.h"
#include "CallStats.h"
#include "ChatMessage.h"
#include "ChatRoom.h"
#include "LpConfig.h"
#include "PayloadType.h"
#include "ProxyConfig.h"
#include "Transports.h"
#include "Utils.h"
#include "VideoPolicy.h"
#include "VideoSize.h"

#include <Stringapiset.h>
#include <time.h>


using namespace Windows::Storage;


std::wstring Linphone::Native::Utils::UTF8ToUTF16(const char *utf8)
{
	if ((utf8 == nullptr) || (*utf8 == '\0'))
		return std::wstring();

	int utf8len = static_cast<int>(strlen(utf8));

	// Get the size to alloc for utf-16 string
	int utf16len = MultiByteToWideChar(CP_UTF8, 0, utf8, utf8len, nullptr, 0);
	if (utf16len == 0) {
		DWORD error = GetLastError();
		ms_error("Invalid UTF-8 character, can't convert to UTF-16: %d", error);
		return std::wstring();
	}

	// Do the conversion
	std::wstring utf16;
	utf16.resize(utf16len);
	if (MultiByteToWideChar(CP_UTF8, 0, utf8, utf8len, &utf16[0], (int)utf16.length()) == 0) {
		DWORD error = GetLastError();
		ms_error("Error during string conversion from UTF-8 to UTF-16: %d", error);
		return std::wstring();
	}
	return utf16;
}

std::string Linphone::Native::Utils::UTF16ToUTF8(const wchar_t *utf16)
{
	if ((utf16 == nullptr) || (*utf16 == L'\0'))
		return std::string();

	// Get the size to alloc for utf-8 string
	int utf16len = static_cast<int>(wcslen(utf16));
	int utf8len = WideCharToMultiByte(CP_UTF8, 0, utf16, utf16len, NULL, 0, NULL, NULL);
	if (utf8len == 0) {
		DWORD error = GetLastError();
		ms_error("Invalid UTF-16 character, can't convert to UTF-8: %d", error);
		return std::string();
	}

	// Do the conversion
	std::string utf8;
	utf8.resize(utf8len);
	if (WideCharToMultiByte(CP_UTF8, 0, utf16, utf16len, &utf8[0], (int)utf8.length(), NULL, NULL) == 0) {
		DWORD error = GetLastError();
		ms_error("Error during string conversion from UTF-16 to UTF-8: %d", error);
		return std::string();
	}
	return utf8;
}

const char* Linphone::Native::Utils::pstoccs(Platform::String^ ps)
{
	if (ps == nullptr || ps->Length() == 0)
		return NULL;

	std::string s = Linphone::Native::Utils::UTF16ToUTF8(ps->Data());
	char* cc = (char*) malloc(s.length()+1);
	memcpy(cc, s.c_str(), s.length());
	cc[s.length()] = '\0';
	return cc;
}

Platform::String^ Linphone::Native::Utils::cctops(const char* cc)
{
	if (cc == NULL)
		return nullptr;

	std::wstring wid_str = Linphone::Native::Utils::UTF8ToUTF16(cc);
	const wchar_t* w_char = wid_str.c_str();
	return ref new Platform::String(w_char);
}



Platform::Object^ Linphone::Native::Utils::CreateAddress(const char *address)
{
	Address^ addr = ref new Linphone::Native::Address(address);
	if (addr->address == nullptr) return nullptr;
	return addr;
}

Platform::Object^ Linphone::Native::Utils::CreateAddress(void *address)
{
	return ref new Linphone::Native::Address((::LinphoneAddress *)address);
}

Platform::Object^ Linphone::Native::Utils::CreateAuthInfo(void *auth_info)
{
	return ref new Linphone::Native::AuthInfo((::LinphoneAuthInfo *)auth_info);
}

Platform::Object^ Linphone::Native::Utils::CreateCallStats(void *callStats)
{
	return ref new Linphone::Native::CallStats((::LinphoneCallStats *)callStats);
}

Platform::Object^ Linphone::Native::Utils::CreateCallStats(void *call, int mediaType)
{
	return ref new Linphone::Native::CallStats((::LinphoneCall *)call, (Linphone::Native::MediaType)mediaType);
}

Platform::Object^ Linphone::Native::Utils::CreateLpConfig(void *config)
{
	return ref new Linphone::Native::LpConfig((::LpConfig *)config);
}

Platform::Object^ Linphone::Native::Utils::CreateLpConfig(Platform::String^ configPath, Platform::String^ factoryConfigPath)
{
	return ref new Linphone::Native::LpConfig(configPath, factoryConfigPath);
}

Platform::Object^ Linphone::Native::Utils::CreatePayloadType(void *pt)
{
	return ref new Linphone::Native::PayloadType((::PayloadType *)pt);
}

Platform::Object^ Linphone::Native::Utils::GetCall(void *call)
{
	::LinphoneCall *lCall = (::LinphoneCall *)call;
	if (lCall == nullptr) {
		return nullptr;
	}
	if (linphone_call_get_user_data(lCall) == nullptr) {
		return ref new Linphone::Native::Call(lCall);
	}
	RefToPtrProxy<Linphone::Native::Call^> *proxy = reinterpret_cast<RefToPtrProxy<Linphone::Native::Call^> *>(linphone_call_get_user_data(lCall));
	return proxy->Ref();
}

Platform::Object^ Linphone::Native::Utils::GetCallLog(void *callLog)
{
	::LinphoneCallLog *lCallLog = (::LinphoneCallLog *)callLog;
	if (lCallLog == nullptr) {
		return nullptr;
	}
	if (linphone_call_log_get_user_data(lCallLog) == nullptr) {
		return ref new Linphone::Native::CallLog(lCallLog);
	}
	RefToPtrProxy<Linphone::Native::CallLog^> *proxy = reinterpret_cast<RefToPtrProxy<Linphone::Native::CallLog^> *>(linphone_call_log_get_user_data(lCallLog));
	return proxy->Ref();
}

Platform::Object^ Linphone::Native::Utils::GetCallParams(void *callParams)
{
	::LinphoneCallParams *lCallParams = (::LinphoneCallParams *)callParams;
	if (lCallParams == nullptr) {
		return nullptr;
	}
	if (linphone_call_params_get_user_data(lCallParams) == nullptr) {
		return ref new Linphone::Native::CallParams(lCallParams);
	}
	RefToPtrProxy<Linphone::Native::CallParams^> *proxy = reinterpret_cast<RefToPtrProxy<Linphone::Native::CallParams^> *>(linphone_call_params_get_user_data(lCallParams));
	return proxy->Ref();
}

Platform::Object^ Linphone::Native::Utils::GetChatMessage(void *message)
{
	::LinphoneChatMessage *lChatMessage = (::LinphoneChatMessage *)message;
	if (lChatMessage == nullptr) {
		return nullptr;
	}
	if (linphone_chat_message_get_user_data(lChatMessage) == nullptr) {
		return ref new Linphone::Native::ChatMessage(lChatMessage);
	}
	RefToPtrProxy<Linphone::Native::ChatMessage^> *proxy = reinterpret_cast<RefToPtrProxy<Linphone::Native::ChatMessage^> *>(linphone_chat_message_get_user_data(lChatMessage));
	return proxy->Ref();
}

Platform::Object^ Linphone::Native::Utils::GetChatRoom(void *room)
{
	::LinphoneChatRoom *lChatRoom = (::LinphoneChatRoom *)room;
	if (lChatRoom == nullptr) {
		return nullptr;
	}
	if (linphone_chat_room_get_user_data(lChatRoom) == nullptr) {
		return ref new Linphone::Native::ChatRoom(lChatRoom);
	}
	RefToPtrProxy<Linphone::Native::ChatRoom^> *proxy = reinterpret_cast<RefToPtrProxy<Linphone::Native::ChatRoom^> *>(linphone_chat_room_get_user_data(lChatRoom));
	return proxy->Ref();
}

Platform::Object^ Linphone::Native::Utils::GetCore(void *core)
{
	::LinphoneCore *lCore = (::LinphoneCore *)core;
	if (linphone_core_get_user_data(lCore) == nullptr) {
		return nullptr;
	}
	RefToPtrProxy<Linphone::Native::Core^> *proxy = reinterpret_cast<RefToPtrProxy<Linphone::Native::Core^> *>(linphone_core_get_user_data(lCore));
	return proxy->Ref();
}

Platform::Object^ Linphone::Native::Utils::GetProxyConfig(void *proxy_config)
{
	::LinphoneProxyConfig *lProxyConfig = (::LinphoneProxyConfig *)proxy_config;
	if (lProxyConfig == nullptr) {
		return nullptr;
	}
	if (linphone_proxy_config_get_user_data(lProxyConfig) == nullptr) {
		return ref new Linphone::Native::ProxyConfig(lProxyConfig);
	}
	RefToPtrProxy<Linphone::Native::ProxyConfig^> *proxy = reinterpret_cast<RefToPtrProxy<Linphone::Native::ProxyConfig^> *>(linphone_proxy_config_get_user_data(lProxyConfig));
	return proxy->Ref();
}

void Linphone::Native::Utils::SetLogLevel(int loglevel)
{
	API_LOCK;
	linphone_core_set_log_level(static_cast<OrtpLogLevel>(loglevel));
}

#if 0
void Linphone::Native::Utils::EchoCalibrationCallback(void *lc, int status, int delay_ms, void *data)
{
	API_LOCK;
	EchoCalibrationData *ecData = static_cast<EchoCalibrationData *>(data);
	if (ecData != nullptr) {
		delete ecData;
	}
	Linphone::Native::RefToPtrProxy<Linphone::Native::LinphoneCore^> *proxy = reinterpret_cast< Linphone::Native::RefToPtrProxy<Linphone::Native::LinphoneCore^> *>(linphone_core_get_user_data(static_cast<::LinphoneCore *>(lc)));
	Linphone::Native::LinphoneCore^ lCore = (proxy) ? proxy->Ref() : nullptr;
	Linphone::Native::EcCalibratorStatus ecStatus = (Linphone::Native::EcCalibratorStatus) status;
	lCore->listener->EcCalibrationStatus(ecStatus, delay_ms);
}
#endif
