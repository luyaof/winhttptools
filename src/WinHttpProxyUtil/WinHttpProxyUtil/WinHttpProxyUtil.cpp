// WinHttpProxyUtil.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

#pragma comment(lib, "winhttp.lib")

#define PROXY_TYPE_MANUAL 0
#define PROXY_TYPE_AUTO 1
#define PROXY_TYPE_NO 3

void GetDefaultProxyConfig()
{
	cout << "+++++++++++ Loading Default Proxy Config ++++++++++" << endl;
	WINHTTP_PROXY_INFO         ProxyInfo;
	// Setup the default proxy

	WinHttpGetDefaultProxyConfiguration(&ProxyInfo);
	cout << "WinHTTP Default Proxy Info:" << endl;

	if (ProxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NO_PROXY)
	{
		cout << "	No Proxy configured" << endl;
	}
	else if (ProxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_DEFAULT_PROXY)
	{

		wcout << "	Proxy Server: " << ProxyInfo.lpszProxy << endl;
		if (ProxyInfo.lpszProxyBypass != NULL)
			wcout << "	Proxy Bypass List: " << ProxyInfo.lpszProxyBypass << endl;
	}

	cout << "IE Config For Current User:" << endl;
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig;
	WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig);


	if (ieProxyConfig.fAutoDetect)
	{
		cout << "	Auto Detect Status: enabled" << endl;
	}
	else
	{
		cout << "	Auto Detect Status: disabled" << endl;
	}


	cout << "	Auto Config URL: ";
	if (ieProxyConfig.lpszAutoConfigUrl)
	{
		cout << ieProxyConfig.lpszAutoConfigUrl << endl;
		GlobalFree(ieProxyConfig.lpszAutoConfigUrl);
	}
	else {
		cout << "Not Configured" << endl;
	}


	cout << "	Proxy Server: ";
	if (ieProxyConfig.lpszProxy)
	{
		cout << ieProxyConfig.lpszProxy << endl;
		GlobalFree(ieProxyConfig.lpszProxy);
	}
	else {
		cout << "Not Configured" << endl;
	}


	cout << "	Proxy Bypass: ";

	if (ieProxyConfig.lpszProxyBypass)
	{
		cout << ieProxyConfig.lpszProxyBypass << endl;
		GlobalFree(ieProxyConfig.lpszProxyBypass);
	}
	else {
		cout << "Not Configured" << endl;
	}
}

void GetDefaultProxyForUrl(wchar_t* lpUrl)
{
	HINTERNET hHttpSession = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;

	WINHTTP_AUTOPROXY_OPTIONS  AutoProxyOptions;
	WINHTTP_PROXY_INFO         ProxyInfo;
	DWORD                      cbProxyInfoSize = sizeof(ProxyInfo);

	ZeroMemory(&AutoProxyOptions, sizeof(AutoProxyOptions));
	ZeroMemory(&ProxyInfo, sizeof(ProxyInfo));

	//
	// Create the WinHTTP session.
	//
	hHttpSession = WinHttpOpen(L"WinHTTP AutoProxy Sample/1.0",
		WINHTTP_ACCESS_TYPE_NO_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);

	// Exit if WinHttpOpen failed.
	if (!hHttpSession)
		goto Exit;

	//
	// Create the WinHTTP connect handle.
	//
	hConnect = WinHttpConnect(hHttpSession,
		lpUrl,
		INTERNET_DEFAULT_HTTP_PORT,
		0);

	// Exit if WinHttpConnect failed.
	if (!hConnect)
		goto Exit;


	// Setup the default proxy
	WinHttpGetDefaultProxyConfiguration(&ProxyInfo);
	printf("Default Proxy Info:\n");
	wcout << "Proxy Server: " << ProxyInfo.lpszProxy << endl;
	if (ProxyInfo.lpszProxyBypass != NULL)
		wcout << "Proxy Bypass List: " << ProxyInfo.lpszProxyBypass << endl;



Exit:
	//
	// Clean up the WINHTTP_PROXY_INFO structure.
	//
	if (ProxyInfo.lpszProxy != NULL)
		GlobalFree(ProxyInfo.lpszProxy);

	if (ProxyInfo.lpszProxyBypass != NULL)
		GlobalFree(ProxyInfo.lpszProxyBypass);

	//
	// Close the WinHTTP handles.
	//
	if (hRequest != NULL)
		WinHttpCloseHandle(hRequest);

	if (hConnect != NULL)
		WinHttpCloseHandle(hConnect);

	if (hHttpSession != NULL)
		WinHttpCloseHandle(hHttpSession);
}

void GetAutoProxy()
{
	HINTERNET hHttpSession = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;

	WINHTTP_AUTOPROXY_OPTIONS  AutoProxyOptions;
	WINHTTP_PROXY_INFO         ProxyInfo;
	DWORD                      cbProxyInfoSize = sizeof(ProxyInfo);

	ZeroMemory(&AutoProxyOptions, sizeof(AutoProxyOptions));
	ZeroMemory(&ProxyInfo, sizeof(ProxyInfo));

	//
	// Create the WinHTTP session.
	//
	hHttpSession = WinHttpOpen(L"WinHTTP AutoProxy Sample/1.0",
		WINHTTP_ACCESS_TYPE_NO_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);

	// Exit if WinHttpOpen failed.
	if (!hHttpSession)
		goto Exit;

	//
	// Create the WinHTTP connect handle.
	//
	hConnect = WinHttpConnect(hHttpSession,
		L"www.microsoft.com",
		INTERNET_DEFAULT_HTTP_PORT,
		0);

	// Exit if WinHttpConnect failed.
	if (!hConnect)
		goto Exit;

	//
	// Create the HTTP request handle.
	//
	hRequest = WinHttpOpenRequest(hConnect,
		L"GET",
		L"ms.htm",
		L"HTTP/1.1",
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		0);

	// Exit if WinHttpOpenRequest failed.
	if (!hRequest)
		goto Exit;

	//
	// Set up the autoproxy call.
	//

	// Use auto-detection because the Proxy 
	// Auto-Config URL is not known.
	AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;

	// Use DHCP and DNS-based auto-detection.
	AutoProxyOptions.dwAutoDetectFlags =
		WINHTTP_AUTO_DETECT_TYPE_DHCP |
		WINHTTP_AUTO_DETECT_TYPE_DNS_A;

	// If obtaining the PAC script requires NTLM/Negotiate
	// authentication, then automatically supply the client
	// domain credentials.
	AutoProxyOptions.fAutoLogonIfChallenged = TRUE;

	//
	// Call WinHttpGetProxyForUrl with our target URL. If 
	// auto-proxy succeeds, then set the proxy info on the 
	// request handle. If auto-proxy fails, ignore the error 
	// and attempt to send the HTTP request directly to the 
	// target server (using the default WINHTTP_ACCESS_TYPE_NO_PROXY 
	// configuration, which the requesthandle will inherit 
	// from the session).
	//
	if (WinHttpGetProxyForUrl(hHttpSession,
		L"http://bing.com",
		&AutoProxyOptions,
		&ProxyInfo))
	{
		// A proxy configuration was found, set it on the
		// request handle.

		if (!WinHttpSetOption(hRequest,
			WINHTTP_OPTION_PROXY,
			&ProxyInfo,
			cbProxyInfoSize))
		{
			// Exit if setting the proxy info failed.
			goto Exit;
		}
	}

	//
	// Send the request.
	//
	if (!WinHttpSendRequest(hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0,
		WINHTTP_NO_REQUEST_DATA,
		0,
		0,
		NULL))
	{
		// Exit if WinHttpSendRequest failed.
		goto Exit;
	}

	//
	// Wait for the response.
	//

	if (!WinHttpReceiveResponse(hRequest, NULL))
		goto Exit;

	//
	// A response has been received, then process it.
	// (omitted)
	//


Exit:
	//
	// Clean up the WINHTTP_PROXY_INFO structure.
	//
	if (ProxyInfo.lpszProxy != NULL)
		GlobalFree(ProxyInfo.lpszProxy);

	if (ProxyInfo.lpszProxyBypass != NULL)
		GlobalFree(ProxyInfo.lpszProxyBypass);

	//
	// Close the WinHTTP handles.
	//
	if (hRequest != NULL)
		WinHttpCloseHandle(hRequest);

	if (hConnect != NULL)
		WinHttpCloseHandle(hConnect);

	if (hHttpSession != NULL)
		WinHttpCloseHandle(hHttpSession);
}

void GetProxyByPacFile(char** argv)
{
	HINTERNET hHttpSession = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;

	WINHTTP_AUTOPROXY_OPTIONS  AutoProxyOptions;
	WINHTTP_PROXY_INFO         ProxyInfo;
	DWORD                      cbProxyInfoSize = sizeof(ProxyInfo);

	bool resultOK = false;
	int wideLengthUrl = MultiByteToWideChar(CP_ACP, 0, argv[1], -1, NULL, 0);
	wchar_t* lpUrl = new WCHAR[wideLengthUrl];
	MultiByteToWideChar(CP_ACP, 0, argv[1], -1, lpUrl, wideLengthUrl);
	int wideLengthProxy = MultiByteToWideChar(CP_ACP, 0, argv[2], -1, NULL, 0);
	wchar_t* proxyUrl = new WCHAR[wideLengthProxy];

	MultiByteToWideChar(CP_ACP, 0, argv[2], -1, proxyUrl, wideLengthProxy);


	ZeroMemory(&AutoProxyOptions, sizeof(AutoProxyOptions));
	ZeroMemory(&ProxyInfo, sizeof(ProxyInfo));

	wcout << "URL: " << lpUrl << endl;
	wcout << "Pac: " << proxyUrl << endl;

	//
	// Create the WinHTTP session.
	//
	hHttpSession = WinHttpOpen(L"WinHTTP AutoProxy Sample/1.0",
		WINHTTP_ACCESS_TYPE_NO_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);

	// Exit if WinHttpOpen failed.
	if (!hHttpSession)
		goto Exit;

	cout << "WinHTTP Session created" << endl;



	wcout << L"Getting Proxy for URL: " << lpUrl << endl;
	// Setup the Proxy Config URL
	AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
	AutoProxyOptions.lpszAutoConfigUrl = proxyUrl;

	resultOK = WinHttpGetProxyForUrl(hHttpSession, lpUrl,
		&AutoProxyOptions, &ProxyInfo
	);

	if (resultOK)
	{
		wcout << L"Proxy Access Type: " << ProxyInfo.dwAccessType << endl;
		if (ProxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NAMED_PROXY)
		{
			wcout << lpUrl << L" -> " << ProxyInfo.lpszProxy << endl;
		}
		else if (ProxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NO_PROXY)
			wcout << lpUrl << L" -> DIRECT" << endl;
	}
	else
	{
		wcout << L"GetProxyForUrl failed: " << GetLastError() << endl;

	}

Exit:
	//
	// Clean up the WINHTTP_PROXY_INFO structure.
	//
	if (ProxyInfo.lpszProxy != NULL)
		GlobalFree(ProxyInfo.lpszProxy);

	if (ProxyInfo.lpszProxyBypass != NULL)
		GlobalFree(ProxyInfo.lpszProxyBypass);

	//
	// Close the WinHTTP handles.
	//
	if (hRequest != NULL)
		WinHttpCloseHandle(hRequest);

	if (hConnect != NULL)
		WinHttpCloseHandle(hConnect);

	if (hHttpSession != NULL)
		WinHttpCloseHandle(hHttpSession);

}

//The handler of user input when 2 parameters specified and second one is the URL
void GetProxyForUrl(char** argv)
{
	int wideLengthUrl = MultiByteToWideChar(CP_ACP, 0, argv[1], -1, NULL, 0);
	LPWSTR lpUrl = new WCHAR[wideLengthUrl];
	MultiByteToWideChar(CP_ACP, 0, argv[1], -1, lpUrl, wideLengthUrl);
	wcout << "+++++++++++ Getting Proxy for URL: " << lpUrl << " ++++++++++" << endl;

	GetDefaultProxyForUrl(lpUrl);
}

void DisplayHelp()
{

}

void ReadOpt(int argc, char** argv)
{
	int i = 1;

	switch (argc)
	{
	case 1:
		GetDefaultProxyConfig();
		break;
	case 2:
		//specify a URL and getproxyforurl using different API
		GetProxyForUrl(argv);
		break;
	case 3:
		GetProxyByPacFile(argv);
		break;
	default:
		cout << "invalid parameter" << endl;
		break;
	}

	//int wideLengthUrl = MultiByteToWideChar(CP_ACP, 0, argv[1], -1, NULL, 0);
	//LPWSTR lpUrl = new WCHAR[wideLengthUrl];
	//MultiByteToWideChar(CP_ACP, 0, argv[1], -1, lpUrl, wideLengthUrl);
	//int wideLengthProxy = MultiByteToWideChar(CP_ACP, 0, argv[2], -1, NULL, 0);
	//LPWSTR lpProxyFile = new WCHAR[wideLengthProxy];
	//MultiByteToWideChar(CP_ACP, 0, argv[2], -1, lpProxyFile, wideLengthProxy);
	//
	//cout << wideLengthUrl << endl;
	//wcout << lpUrl;
	//cout << endl;
	//GetProxyByPacFile(lpUrl, lpProxyFile);
}

int main(int argc, char** argv)
{
	ReadOpt(argc, argv);
	//GetDefaultProxyForUrl();
	return 0;
}