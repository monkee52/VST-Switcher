#define UNICODE
#define _UNICODE

#include <Windows.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <MsXml6.h>
#include <comdef.h>
#include <math.h>

#include "PolicyConfig.h"

#define SAFE_RELEASE(punk) if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }
#define CHK_ALLOC(p) ((!(p)) ? E_OUTOFMEMORY : S_OK)

#define PING() wprintf(L"file: %hs line: %d\n", __FILE__, __LINE__)

HRESULT VariantFromString(PCWSTR wszValue, VARIANT &variant) {
	HRESULT hr = S_OK;
	BSTR bstr = SysAllocString(wszValue);

	if (!bstr) {
		return E_OUTOFMEMORY;
	}

	V_VT(&variant) = VT_BSTR;
	V_BSTR(&variant) = bstr;

	return hr;
}

LPWSTR ConvHrToWstr(HRESULT hr) {
	_com_error err(hr);

	LPWSTR text = (LPWSTR)err.ErrorMessage();
	DWORD len = 13 + wcslen(text);
	LPWSTR result = new WCHAR[len];

	_snwprintf(result, len, L"%#010x: %s", hr, text);

	return result;
}

LPWSTR ConvWinErrToWstr() {
	DWORD errCode = GetLastError();

	LPWSTR text;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&text, 0, NULL);

	DWORD len = 13 + wcslen(text);
	LPWSTR result = new WCHAR[len];

	_snwprintf(result, len, L"%#010x: %s", errCode, text);

	LocalFree(text);

	return result;
}

class Configuration {
	private:
		IXMLDOMDocument * pXMLDom = NULL;
		bool unsavedChanges = false;
		bool opened = false;
		LPWSTR filename = NULL;

	public:
		Configuration(LPWSTR filename) {
			this->filename = filename;
		}

		~Configuration() {
			SAFE_RELEASE(this->pXMLDom);

			this->opened = false;
		}

		void Open() {
			if (this->opened) {
				return;
			}

			HRESULT hr = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&this->pXMLDom));

			if (FAILED(hr)) {
				throw ConvHrToWstr(hr);
			}

			this->pXMLDom->put_async(VARIANT_FALSE);
			this->pXMLDom->put_validateOnParse(VARIANT_TRUE);
			this->pXMLDom->put_resolveExternals(VARIANT_FALSE);

			VARIANT varFilename;

			VariantInit(&varFilename);

			hr = VariantFromString(this->filename, varFilename);

			if (FAILED(hr)) {
				SAFE_RELEASE(this->pXMLDom);
				
				VariantClear(&varFilename);

				throw ConvHrToWstr(hr);
			}

			VARIANT_BOOL varStatus;

			hr = pXMLDom->load(varFilename, &varStatus);

			VariantClear(&varFilename);

			if (FAILED(hr)) {
				SAFE_RELEASE(this->pXMLDom);

				throw ConvHrToWstr(hr);
			}

			if (varStatus != VARIANT_TRUE) {
				IXMLDOMParseError * pXMLErr = NULL;

				hr = pXMLDom->get_parseError(&pXMLErr);

				if (FAILED(hr)) {
					SAFE_RELEASE(this->pXMLDom);

					throw ConvHrToWstr(hr);
				}

				long errCode;

				hr = pXMLErr->get_errorCode(&errCode);

				if (FAILED(hr)) {
					SAFE_RELEASE(pXMLErr);
					SAFE_RELEASE(this->pXMLDom);

					throw ConvHrToWstr(hr);
				}

				BSTR bstrErr;

				hr = pXMLErr->get_reason(&bstrErr);

				if (FAILED(hr)) {
					SAFE_RELEASE(pXMLErr);
					SAFE_RELEASE(this->pXMLDom);

					throw ConvHrToWstr(hr);
				}

				DWORD len = 13 + wcslen(bstrErr);
				LPWSTR errMsg = new WCHAR[len];

				_snwprintf(errMsg, len, L"%#010x: %s", errCode, bstrErr);

				SysFreeString(bstrErr);

				throw errMsg;
			}

			this->opened = true;
		}

		void Save() {
			if (!this->unsavedChanges) {
				return;
			}

			VARIANT varFilename;

			VariantInit(&varFilename);

			HRESULT hr = VariantFromString(this->filename, varFilename);

			if (FAILED(hr)) {
				SAFE_RELEASE(this->pXMLDom);

				VariantClear(&varFilename);
			}

			hr = this->pXMLDom->save(varFilename);

			if (FAILED(hr)) {
				SAFE_RELEASE(this->pXMLDom);

				throw hr;
			}

			this->unsavedChanges = false;
		}

		LPWSTR Read(LPWSTR key, LPWSTR defaultValue) {
			DWORD queryLen = wcslen(key) + 27;
			LPWSTR query = new WCHAR[queryLen];

			_snwprintf(query, queryLen, L"/settings/setting[@key='%s']", key);

			BSTR bstrQuery = SysAllocString(query);

			delete query;

			HRESULT hr = CHK_ALLOC(bstrQuery);

			if (FAILED(hr)) {
				throw ConvHrToWstr(hr);
			}

			IXMLDOMNode * pNode = NULL;
			IXMLDOMNodeList * pXMLNodeList = NULL;

			hr = this->pXMLDom->selectSingleNode(bstrQuery, &pNode);

			SysFreeString(bstrQuery);

			if (FAILED(hr)) {
				throw ConvHrToWstr(hr);
			}

			if (pNode == NULL) {
				return defaultValue;
			}

			hr = pNode->get_childNodes(&pXMLNodeList);

			SAFE_RELEASE(pNode);

			if (FAILED(hr)) {
				throw ConvHrToWstr(hr);
			}

			long lCount;

			hr = pXMLNodeList->get_length(&lCount);

			if (FAILED(hr)) {
				SAFE_RELEASE(pXMLNodeList);

				throw ConvHrToWstr(hr);
			}

			if (lCount > 0) {
				hr = pXMLNodeList->get_item(0, &pNode);

				SAFE_RELEASE(pXMLNodeList);

				if (FAILED(hr)) {
					throw ConvHrToWstr(hr);
				}

				BSTR bstrNodeValue = NULL;

				hr = pNode->get_text(&bstrNodeValue);

				if (FAILED(hr)) {
					throw ConvHrToWstr(hr);
				}

				LPWSTR retValue = bstrNodeValue == NULL ? L"" : bstrNodeValue;

				SAFE_RELEASE(pNode);

				return retValue;
			} else {
				SAFE_RELEASE(pXMLNodeList);

				return defaultValue;
			}
		}

		void Write(LPWSTR key, LPWSTR value) {
			this->unsavedChanges = true;

			DWORD queryLen = wcslen(key) + 27;
			LPWSTR query = new WCHAR[queryLen];

			_snwprintf(query, queryLen, L"/settings/setting[@key='%s']", key);

			BSTR bstrQuery = SysAllocString(query);

			delete query;

			HRESULT hr = CHK_ALLOC(bstrQuery);

			if (FAILED(hr)) {
				throw ConvHrToWstr(hr);
			}

			IXMLDOMNode * pNode = NULL;
			IXMLDOMNodeList * pXMLNodeList = NULL;

			hr = this->pXMLDom->selectSingleNode(bstrQuery, &pNode);

			SysFreeString(bstrQuery);

			if (FAILED(hr)) {
				throw ConvHrToWstr(hr);
			}

			if (pNode == NULL) {
				IXMLDOMElement * pElement = NULL;

				BSTR bstrName = SysAllocString(L"setting");

				hr = this->pXMLDom->createElement(bstrName, &pElement);

				SysFreeString(bstrName);

				if (FAILED(hr)) {
					throw ConvHrToWstr(hr);
				}

				IXMLDOMAttribute * pAttribute = NULL;

				bstrName = SysAllocString(L"key");

				hr = this->pXMLDom->createAttribute(bstrName, &pAttribute);

				if (FAILED(hr)) {
					SAFE_RELEASE(pElement);

					SysFreeString(bstrName);

					throw ConvHrToWstr(hr);
				}

				BSTR bstrValue = SysAllocString(value);
				VARIANT varValue;

				VariantInit(&varValue);
				VariantFromString(value, varValue);

				hr = pAttribute->put_value(varValue);

				VariantClear(&varValue);
				SysFreeString(bstrValue);

				if (FAILED(hr)) {
					SAFE_RELEASE(pElement);
					SAFE_RELEASE(pAttribute);

					throw ConvHrToWstr(hr);
				}

				IXMLDOMAttribute * pAttributeOut = NULL;

				hr = pElement->setAttributeNode(pAttribute, &pAttributeOut);
				
				SAFE_RELEASE(pElement);
				SAFE_RELEASE(pAttribute);
				SAFE_RELEASE(pAttributeOut);

				if (FAILED(hr)) {
					throw ConvHrToWstr(hr);
				}

				IXMLDOMElement * pParent = NULL;

				hr = this->pXMLDom->get_documentElement(&pParent);

				if (FAILED(hr)) {
					throw ConvHrToWstr(hr);
				}

				IXMLDOMNode * pChildOut;

				hr = pParent->appendChild(pElement, &pChildOut);

				SAFE_RELEASE(pElement);
				SAFE_RELEASE(pChildOut);
				SAFE_RELEASE(pParent);

				if (FAILED(hr)) {
					throw ConvHrToWstr(hr);
				}

				return;
			}

			hr = pNode->get_childNodes(&pXMLNodeList);

			if (FAILED(hr)) {
				SAFE_RELEASE(pNode);

				throw ConvHrToWstr(hr);
			}

			long lCount;

			hr = pXMLNodeList->get_length(&lCount);

			if (FAILED(hr)) {
				SAFE_RELEASE(pNode);
				SAFE_RELEASE(pXMLNodeList);

				throw ConvHrToWstr(hr);
			}

			if (lCount > 0) {
				SAFE_RELEASE(pNode);

				hr = pXMLNodeList->get_item(0, &pNode);

				SAFE_RELEASE(pXMLNodeList);

				if (FAILED(hr)) {
					throw ConvHrToWstr(hr);
				}

				BSTR bstrNodeValue = SysAllocString(value);
				
				hr = pNode->put_text(bstrNodeValue);

				SysFreeString(bstrNodeValue);

				if (FAILED(hr)) {
					throw ConvHrToWstr(hr);
				}

				SAFE_RELEASE(pNode);
			} else {
				SAFE_RELEASE(pXMLNodeList);

				IXMLDOMText * pText = NULL;

				BSTR bstrNodeValue = SysAllocString(value);

				hr = this->pXMLDom->createTextNode(bstrNodeValue, &pText);

				SysFreeString(bstrNodeValue);

				if (FAILED(hr)) {
					throw ConvHrToWstr(hr);
				}

				IXMLDOMNode * pChildOut = NULL;

				hr = pNode->appendChild(pText, &pChildOut);

				SAFE_RELEASE(pNode);
				SAFE_RELEASE(pText);
				SAFE_RELEASE(pChildOut);

				if (FAILED(hr)) {
					throw ConvHrToWstr(hr);
				}
			}
		}
};

LPWSTR GetAudioEndpointId(LPWSTR name) {
	IMMDeviceEnumerator * pEnumerator = NULL;

	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID *)&pEnumerator);

	if (FAILED(hr)) {
		throw ConvHrToWstr(hr);
	}

	IMMDevice * pEndpoint = NULL;
	LPWSTR retValue = NULL;

	// Return default device ID
	if (name == NULL) {
		hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pEndpoint);

		if (FAILED(hr)) {
			SAFE_RELEASE(pEnumerator);

			throw ConvHrToWstr(hr);
		}

		LPWSTR pwszID = NULL;

		hr = pEndpoint->GetId(&pwszID);

		if (FAILED(hr)) {
			SAFE_RELEASE(pEnumerator);
			SAFE_RELEASE(pEndpoint);

			throw ConvHrToWstr(hr);
		}

		retValue = new WCHAR[wcslen(pwszID) + 1];

		wcscpy(retValue, pwszID);

		SAFE_RELEASE(pEnumerator);
		SAFE_RELEASE(pEndpoint);

		return retValue;
	}

	IMMDeviceCollection * pCollection = NULL;

	hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);

	SAFE_RELEASE(pEnumerator);

	if (FAILED(hr)) {
		throw ConvHrToWstr(hr);
	}

	UINT count;

	hr = pCollection->GetCount(&count);

	if (FAILED(hr)) {
		SAFE_RELEASE(pCollection);

		throw ConvHrToWstr(hr);
	}

	if (count == 0) {
		SAFE_RELEASE(pCollection);

		return NULL;
	}

	for (ULONG i = 0; i < count; i++) {
		IMMDevice * pEndpoint = NULL;

		hr = pCollection->Item(i, &pEndpoint);

		if (FAILED(hr)) {
			SAFE_RELEASE(pCollection);

			throw ConvHrToWstr(hr);
		}

		LPWSTR pwszID;

		hr = pEndpoint->GetId(&pwszID);

		if (FAILED(hr)) {
			SAFE_RELEASE(pCollection);
			SAFE_RELEASE(pEndpoint);

			throw ConvHrToWstr(hr);
		}

		IPropertyStore * pProps = NULL;

		hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);

		if (FAILED(hr)) {
			SAFE_RELEASE(pCollection);
			SAFE_RELEASE(pEndpoint);

			throw ConvHrToWstr(hr);
		}

		PROPVARIANT varName;

		PropVariantInit(&varName);

		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);

		if (FAILED(hr)) {
			SAFE_RELEASE(pCollection);
			SAFE_RELEASE(pEndpoint);
			SAFE_RELEASE(pProps);
			
			PropVariantClear(&varName);

			throw ConvHrToWstr(hr);
		}

		if (wcscmp(varName.pwszVal, name) == 0) {
			retValue = new WCHAR[wcslen(pwszID) + 1];

			wcscpy(retValue, pwszID);

			SAFE_RELEASE(pCollection);
			SAFE_RELEASE(pEndpoint);
			SAFE_RELEASE(pProps);

			PropVariantClear(&varName);
			CoTaskMemFree(pwszID);

			return retValue;
		}

		SAFE_RELEASE(pEndpoint);
		SAFE_RELEASE(pProps);

		CoTaskMemFree(pwszID);
	}

	SAFE_RELEASE(pCollection);

	return NULL;
}

void SetDefaultAudioEndpoint(LPWSTR deviceId) {
	IPolicyConfigVista * pPolicyConfig = NULL;

	HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfig);

	if (FAILED(hr)) {
		throw ConvHrToWstr(hr);
	}

	hr = pPolicyConfig->SetDefaultEndpoint(deviceId, eMultimedia);

	SAFE_RELEASE(pPolicyConfig);

	if (FAILED(hr)) {
		throw ConvHrToWstr(hr);
	}
}

void RwAudioEndpointVolume(LPWSTR deviceId, BOOL set, float * volume, BOOL * mute) {
	IMMDeviceEnumerator * pEnumerator = NULL;

	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID *)&pEnumerator);

	if (FAILED(hr)) {
		throw ConvHrToWstr(hr);
	}

	IMMDevice * pEndpoint = NULL;

	hr = pEnumerator->GetDevice(deviceId, &pEndpoint);

	SAFE_RELEASE(pEnumerator);

	if (FAILED(hr)) {
		throw ConvHrToWstr(hr);
	}

	IAudioEndpointVolume * pEndpointVolume = NULL;

	hr = pEndpoint->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&pEndpointVolume);

	SAFE_RELEASE(pEndpoint);

	if (FAILED(hr)) {
		throw ConvHrToWstr(hr);
	}

	if (set) {
		hr = pEndpointVolume->SetMasterVolumeLevelScalar(*volume, NULL);
	} else {
		hr = pEndpointVolume->GetMasterVolumeLevelScalar(volume);
	}

	if (FAILED(hr)) {
		SAFE_RELEASE(pEndpointVolume);

		throw ConvHrToWstr(hr);
	}

	if (set) {
		hr = pEndpointVolume->SetMute(*mute, NULL);
	} else {
		hr = pEndpointVolume->GetMute(mute);
	}

	SAFE_RELEASE(pEndpointVolume);

	if (FAILED(hr)) {
		throw ConvHrToWstr(hr);
	}
}

void GetAudioEndpointVolume(LPWSTR deviceId, float * volume, BOOL * mute) {
	RwAudioEndpointVolume(deviceId, FALSE, volume, mute);
}

void SetAudioEndpointVolume(LPWSTR deviceId, float volume, BOOL mute) {
	volume = max(0.0, min(1.0, volume));

	RwAudioEndpointVolume(deviceId, TRUE, &volume, &mute);
}

void LaunchAndWait(LPWSTR programPath) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));

	si.cb = sizeof(si);

	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL, programPath, NULL, NULL, FALSE, 0, NULL, NULL, (LPSTARTUPINFO)&si, (LPPROCESS_INFORMATION)&pi)) {
		throw ConvWinErrToWstr();
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

int wmain(int argc, wchar_t * argv[]) {
	HRESULT hr = CoInitialize(NULL);

	if (FAILED(hr)) {
		fwprintf(stderr, L"Could not initialize COM\n %s\n", ConvHrToWstr(hr));

		return EXIT_FAILURE;
	}

	Configuration * config = new Configuration(L"config.xml");

	try {
		config->Open();
	} catch (LPWSTR reason) {
		fwprintf(stderr, L"The configuration could not be opened.\n %s\n", reason);

		delete config;

		return EXIT_FAILURE;
	}

	LPWSTR defaultDeviceName = config->Read(L"device.default", NULL);
	LPWSTR targetDeviceName = config->Read(L"device.alt", NULL);
	LPWSTR programPath = config->Read(L"program", NULL);

	delete config;

	if (targetDeviceName == NULL) {
		fwprintf(stderr, L"The secondary device is not configured.\n");

		return EXIT_FAILURE;
	}

	if (programPath == NULL) {
		fwprintf(stderr, L"The program path is not configured.\n");
	
		return EXIT_FAILURE;
	}

	LPWSTR currentDeviceId = GetAudioEndpointId(NULL);
	LPWSTR defaultDeviceId = GetAudioEndpointId(defaultDeviceName);
	LPWSTR targetDeviceId = GetAudioEndpointId(targetDeviceName);

	if (defaultDeviceId == NULL) {
		fwprintf(stderr, L"Could not find default device ID.\n");

		return EXIT_FAILURE;
	}

	if (targetDeviceId == NULL) {
		fwprintf(stderr, L"Could not find target device ID.\n");

		return EXIT_FAILURE;
	}

	bool currentIsDefault = wcscmp(currentDeviceId, defaultDeviceId) == 0;

	delete currentDeviceId;

	if (!currentIsDefault) {
		fwprintf(stderr, L"The current audio endpoint is not the default.\n");

		return EXIT_FAILURE;
	}

	bool defaultIsTarget = wcscmp(defaultDeviceId, targetDeviceId) == 0;

	if (defaultIsTarget) {
		fwprintf(stderr, L"The current audio endpoint is the same as the target audio endpoint.\n");

		return EXIT_FAILURE;
	}

	float volume;
	BOOL mute;

	GetAudioEndpointVolume(defaultDeviceId, &volume, &mute);
	SetAudioEndpointVolume(defaultDeviceId, (float)0.0, TRUE);

	// Match volume
	// TODO:
	// * Custom functions
	SetAudioEndpointVolume(targetDeviceId, (5.0 * log10(volume) + 10.0) / 13.0, mute);

	SetDefaultAudioEndpoint(targetDeviceId);

	// Sleep to prevent loud pop
	Sleep(500);

	SetAudioEndpointVolume(defaultDeviceId, (float)1.0, FALSE);

	// Prevent accidental closure
	HWND hWnd = GetConsoleWindow();
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);

	EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	// Launch program
	try {
		LaunchAndWait(programPath);
	} catch (LPWSTR err) {
		fwprintf(stderr, L"Error running program.\n %s\n", err);
	}

	// Restore
	EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);

	// Match volume
	// TODO:
	// * Custom functions
	GetAudioEndpointVolume(targetDeviceId, &volume, &mute);

	SetAudioEndpointVolume(defaultDeviceId, pow(10.0, 13.0 * volume / 5.0 - 2.0), mute);
	SetDefaultAudioEndpoint(defaultDeviceId);

	CoUninitialize();

	return EXIT_SUCCESS;
}
