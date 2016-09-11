#include "stdafx.h"

#include "VST.Audio.h"

using namespace System::Runtime::InteropServices;

namespace VST {
	namespace Audio {
		Controller::Controller() {
			HRESULT hr = CoInitialize(NULL);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(String::Format(gcnew String(_T("Could not initialize COM: {0}")), ConvertHrToString(hr)));
			}
		}

		Controller::~Controller() {
			CoUninitialize();
		}

		array<Endpoint^>^ Controller::GetAudioEndpoints() {
			CComPtr<IMMDeviceEnumerator> pEnumerator = nullptr;

			HRESULT hr = pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			CComPtr<IMMDeviceCollection> pCollection = nullptr;

			hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			UINT count;

			hr = pCollection->GetCount(&count);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			array<Endpoint^>^ endpoints = gcnew array<Endpoint^>(count);

			for (UINT i = 0; i < count; i++) {
				CComPtr<IMMDevice> pEndpoint = nullptr;

				hr = pCollection->Item(i, &pEndpoint);

				if (FAILED(hr)) {
					throw gcnew ApplicationException(ConvertHrToString(hr));
				}

				LPWSTR wszId = nullptr;

				hr = pEndpoint->GetId(&wszId);

				if (FAILED(hr)) {
					throw gcnew ApplicationException(ConvertHrToString(hr));
				}

				String^ id = gcnew String(wszId);

				CoTaskMemFree(wszId);

				endpoints[i] = gcnew Endpoint(this, id);
			}

			return endpoints;
		}

		Endpoint^ Controller::GetDefaultAudioEndpoint() {
			CComPtr<IMMDeviceEnumerator> pEnumerator = nullptr;

			HRESULT hr = pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			CComPtr<IMMDevice> pEndpoint = nullptr;

			hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pEndpoint);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			LPWSTR wszId = nullptr;

			hr = pEndpoint->GetId(&wszId);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			String^ id = gcnew String(wszId);

			CoTaskMemFree(wszId);

			return gcnew Endpoint(this, id);
		}

		void Controller::SetDefaultAudioEndpoint(Endpoint^ endpoint) {
			CComPtr<IPolicyConfigVista> pPolicyConfig = nullptr;

			HRESULT hr = pPolicyConfig.CoCreateInstance(__uuidof(CPolicyConfigVistaClient));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			IntPtr hId = Marshal::StringToHGlobalUni(endpoint->Id);
			LPWSTR wszId = (LPWSTR)hId.ToPointer();

			hr = pPolicyConfig->SetDefaultEndpoint(wszId, eMultimedia);

			Marshal::FreeHGlobal(hId);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			// Delay for the device to actually switch over
			Sleep(300);
		}
	}
}
