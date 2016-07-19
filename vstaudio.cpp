#include <stdio.h>
#include <Windows.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <Objbase.h>
#include <PropIdl.h>
#include <endpointvolume.h>

#include "PolicyConfig.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "user32.lib")

#define EXIT_ON_ERROR(hres) if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk) if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }

int main(int argc, char * argv[]) {
	HWND hWnd = GetConsoleWindow();
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);

	EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	LPWSTR targetID = NULL;
	LPWSTR defaultID = NULL;

	bool flag = false;

	IAudioEndpointVolume *pEndpointVolume = NULL;
	IPolicyConfigVista *pPolicyConfig = NULL;

	HRESULT hr = S_OK;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDeviceCollection *pCollection = NULL;
	IMMDevice *pEndpoint = NULL;
	IPropertyStore *pProps = NULL;
	LPWSTR pwszID = NULL;

	hr = CoInitialize(NULL);

	EXIT_ON_ERROR(hr);

	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID *)&pEnumerator);

	EXIT_ON_ERROR(hr);

	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pEndpoint);

	EXIT_ON_ERROR(hr);

	hr = pEndpoint->GetId(&pwszID);

	EXIT_ON_ERROR(hr);

	defaultID = (LPWSTR)malloc(wcslen(pwszID) * sizeof(WCHAR) + 1);

	wcscpy(defaultID, pwszID);

	SAFE_RELEASE(pEndpoint);

	hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);

	EXIT_ON_ERROR(hr);

	UINT count;

	hr = pCollection->GetCount(&count);

	EXIT_ON_ERROR(hr);

	if (count == 0) {
		printf("No endpoints found.\n");
		goto Exit;
	}

	for (ULONG i = 0; i < count; i++) {
		hr = pCollection->Item(i, &pEndpoint);

		EXIT_ON_ERROR(hr);

		hr = pEndpoint->GetId(&pwszID);

		EXIT_ON_ERROR(hr);

		hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);

		EXIT_ON_ERROR(hr);

		PROPVARIANT varName;

		PropVariantInit(&varName);

		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);

		EXIT_ON_ERROR(hr);

		if (wcscmp(varName.pwszVal, L"Line 1 (Virtual Audio Cable)") == 0) {
			targetID = (LPWSTR)malloc(wcslen(pwszID) * sizeof(WCHAR) + 1);

			wcscpy(targetID, pwszID);

			flag = true;
		}

		CoTaskMemFree(pwszID);

		pwszID = NULL;

		PropVariantClear(&varName);

		SAFE_RELEASE(pProps);
		SAFE_RELEASE(pEndpoint);

		if (flag) {
			break;
		}
	}

	SAFE_RELEASE(pCollection);

	hr = pEnumerator->GetDevice(defaultID, &pEndpoint);

	EXIT_ON_ERROR(hr);

	hr = pEndpoint->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&pEndpointVolume);

	EXIT_ON_ERROR(hr);
	SAFE_RELEASE(pEndpoint);

	hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfig);

	EXIT_ON_ERROR(hr);

	hr = pPolicyConfig->SetDefaultEndpoint(targetID, eMultimedia);

	EXIT_ON_ERROR(hr);

	free(targetID);

	// Small delay for the system to actually switch over the audio device, prevents loud volume surge
	Sleep(250);

	float defaultVolume = 0.0;

	hr = pEndpointVolume->GetMasterVolumeLevelScalar(&defaultVolume);

	EXIT_ON_ERROR(hr);

	hr = pEndpointVolume->SetMasterVolumeLevelScalar((float)1.0, NULL);

	EXIT_ON_ERROR(hr);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));

	si.cb = sizeof(si);

	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL, "C:\\Users\\Ayden\\VSTHost\\vsthost.exe", NULL, NULL, FALSE, 0, NULL, NULL, (LPSTARTUPINFO)&si, (LPPROCESS_INFORMATION)&pi)) {
		printf("CreateProcess failed (%d).\n", GetLastError());
	} else {
		WaitForSingleObject(pi.hProcess, INFINITE);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	hr = pPolicyConfig->SetDefaultEndpoint(defaultID, eMultimedia);

	EXIT_ON_ERROR(hr);

	free(defaultID);

	SAFE_RELEASE(pPolicyConfig);

	hr = pEndpointVolume->SetMasterVolumeLevelScalar(defaultVolume, NULL);

	EXIT_ON_ERROR(hr);
	SAFE_RELEASE(pEndpointVolume);

	SAFE_RELEASE(pEnumerator);

	CoUninitialize();

	EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);

	return EXIT_SUCCESS;

	Exit:
		printf("Error! System may be in an inconsistent state. Please check.\n");

		CoTaskMemFree(pwszID);

		SAFE_RELEASE(pEnumerator);
		SAFE_RELEASE(pCollection);
		SAFE_RELEASE(pEndpoint);
		SAFE_RELEASE(pProps);

		SAFE_RELEASE(pPolicyConfig);
		SAFE_RELEASE(pEndpointVolume);

		CoUninitialize();

		EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);

		return EXIT_FAILURE;
}
