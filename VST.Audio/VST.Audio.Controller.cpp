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

			GCHandle^ hThis = GCHandle::Alloc(this, GCHandleType::Weak);

			this->notificationClient = new CMMNotificationClient(hThis);

			CComPtr<IMMDeviceEnumerator> pEnumerator = nullptr;

			hr = pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			hr = pEnumerator->RegisterEndpointNotificationCallback(this->notificationClient);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}
		}

		Controller::~Controller() {
			CComPtr<IMMDeviceEnumerator> pEnumerator = nullptr;

			HRESULT hr = pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			hr = pEnumerator->UnregisterEndpointNotificationCallback(this->notificationClient);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			if (this->notificationClient != nullptr) {
				this->notificationClient->Release();
			}

			CoUninitialize();
		}

		Controller::!Controller() {
			delete this;
		}

		void Controller::FireDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDevice) {
			this->OnDefaultDeviceChanged(gcnew Endpoint(this, gcnew String(pwstrDefaultDevice)), nullptr);
		}

		void Controller::FireDeviceAdded(LPCWSTR pwstrDeviceId) {
			this->OnDeviceAdded(gcnew Endpoint(this, gcnew String(pwstrDeviceId)), nullptr);
		}

		void Controller::FireDeviceRemoved(LPCWSTR pwstrDeviceId) {
			this->OnDeviceRemoved(gcnew Endpoint(this, gcnew String(pwstrDeviceId)), nullptr);
		}

		void Controller::FireDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) {
			this->OnDeviceStateChanged(gcnew Endpoint(this, gcnew String(pwstrDeviceId)), nullptr);
		}

		void Controller::FirePropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) {
			this->OnPropertyValueChanged(gcnew Endpoint(this, gcnew String(pwstrDeviceId)), nullptr);
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

				LPWSTR pwszId = nullptr;

				hr = pEndpoint->GetId(&pwszId);

				if (FAILED(hr)) {
					throw gcnew ApplicationException(ConvertHrToString(hr));
				}

				String^ id = gcnew String(pwszId);

				CoTaskMemFree(pwszId);

				endpoints[i] = gcnew Endpoint(this, id);
			}

			return endpoints;
		}

		Endpoint^ Controller::GetAudioEndpoint(String^ id) {
			return gcnew Endpoint(this, id);
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

			LPWSTR pwszId = nullptr;

			hr = pEndpoint->GetId(&pwszId);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			String^ id = gcnew String(pwszId);

			CoTaskMemFree(pwszId);

			return gcnew Endpoint(this, id);
		}

		void Controller::SetDefaultAudioEndpoint(Endpoint^ endpoint) {
			CComPtr<IPolicyConfigVista> pPolicyConfig = nullptr;

			HRESULT hr = pPolicyConfig.CoCreateInstance(__uuidof(CPolicyConfigVistaClient));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			IntPtr hId = Marshal::StringToHGlobalUni(endpoint->Id);
			LPWSTR pwszId = (LPWSTR)hId.ToPointer();

			hr = pPolicyConfig->SetDefaultEndpoint(pwszId, eMultimedia);

			Marshal::FreeHGlobal(hId);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			// Delay for the device to actually switch over
			Sleep(300);
		}
	}
}
