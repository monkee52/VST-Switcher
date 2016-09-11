#include "stdafx.h"

#include "VST.Audio.h"

using namespace System::Runtime::InteropServices;

String^ ConvertHrToString(HRESULT hr) {
	_com_error err(hr);

	LPTSTR text = (LPTSTR)err.ErrorMessage();
	DWORD len = 13 + _tcslen(text);
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
