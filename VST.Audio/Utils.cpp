#include "stdafx.h"

#include "VST.Audio.h"

using namespace System::Runtime::InteropServices;

String^ ConvertHrToString(HRESULT hr) {
	_com_error err(hr);

	LPTSTR text = (LPTSTR)err.ErrorMessage();
	size_t len = 13 + _tcslen(text);
	LPTSTR result = new TCHAR[len];

	_sntprintf(result, len, _T("%#010x: %s"), hr, text);

	String^ netText = gcnew String(result);

	delete result;

	return netText;
}

void VolumeCommon(String^ id, bool set, float* volume, bool* mute) {
	CComPtr<IMMDeviceEnumerator> pEnumerator = nullptr;

	HRESULT hr = pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

	if (FAILED(hr)) {
		throw gcnew ApplicationException(ConvertHrToString(hr));
	}

	CComPtr<IMMDevice> pEndpoint = nullptr;

	IntPtr hId = Marshal::StringToHGlobalUni(id);
	LPWSTR wszId = (LPWSTR)hId.ToPointer();

	hr = pEnumerator->GetDevice(wszId, &pEndpoint);

	Marshal::FreeHGlobal(hId);

	if (FAILED(hr)) {
		throw gcnew ApplicationException(ConvertHrToString(hr));
	}

	CComPtr<IAudioEndpointVolume> pEndpointVolume = nullptr;

	hr = pEndpoint->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID*)&pEndpointVolume);

	if (FAILED(hr)) {
		throw gcnew ApplicationException(ConvertHrToString(hr));
	}

	if (volume != nullptr) {
		if (set) {
			hr = pEndpointVolume->SetMasterVolumeLevelScalar(*volume, NULL);
		} else {
			hr = pEndpointVolume->GetMasterVolumeLevelScalar(volume);
		}

		if (FAILED(hr)) {
			throw gcnew ApplicationException(ConvertHrToString(hr));
		}
	}

	if (mute != nullptr) {
		if (set) {
			hr = pEndpointVolume->SetMute(*mute, NULL);
		} else {
			hr = pEndpointVolume->GetMute((BOOL*)mute);
		}

		if (FAILED(hr)) {
			throw gcnew ApplicationException(ConvertHrToString(hr));
		}
	}
}

String^ GetPropertyCommon(String^ id, const PROPERTYKEY key) {
	CComPtr<IMMDeviceEnumerator> pEnumerator = nullptr;

	HRESULT hr = pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

	if (FAILED(hr)) {
		throw gcnew ApplicationException(ConvertHrToString(hr));
	}

	CComPtr<IMMDevice> pEndpoint = nullptr;

	IntPtr hId = Marshal::StringToHGlobalUni(id);
	LPWSTR szId = (LPWSTR)hId.ToPointer();

	hr = pEnumerator->GetDevice(szId, &pEndpoint);

	Marshal::FreeHGlobal(hId);

	if (FAILED(hr)) {
		throw gcnew ApplicationException(ConvertHrToString(hr));
	}

	if (pEndpoint == nullptr) {
		throw gcnew ApplicationException(String::Format(gcnew String(_T("Unable to find device {0}")), id));
	}

	CComPtr<IPropertyStore> pProps = nullptr;

	hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);

	if (FAILED(hr)) {
		throw gcnew ApplicationException(ConvertHrToString(hr));
	}

	PROPVARIANT varName;

	PropVariantInit(&varName);

	hr = pProps->GetValue(key, &varName);

	if (FAILED(hr)) {
		PropVariantClear(&varName);

		throw gcnew ApplicationException(ConvertHrToString(hr));
	}

	String^ name = gcnew String(varName.pwszVal);

	PropVariantClear(&varName);

	return name;
}
