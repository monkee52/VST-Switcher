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

			CComPtr<IMMDeviceEnumerator> pEnumerator = nullptr;

			hr = pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			GCHandle^ hThis = GCHandle::Alloc(this, GCHandleType::Weak);

			this->notificationClient = new CMMNotificationClient(hThis);

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
			this->OnDefaultDeviceChanged(gcnew Device(this, gcnew String(pwstrDefaultDevice)), gcnew DefaultDeviceChangedEventArgs((DeviceType)flow, (DeviceRole)role));
		}

		void Controller::FireDeviceAdded(LPCWSTR pwstrDeviceId) {
			this->OnDeviceAdded(gcnew Device(this, gcnew String(pwstrDeviceId)), EventArgs::Empty);
		}

		void Controller::FireDeviceRemoved(LPCWSTR pwstrDeviceId) {
			this->OnDeviceRemoved(gcnew Device(this, gcnew String(pwstrDeviceId)), EventArgs::Empty);
		}

		void Controller::FireDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) {
			this->OnDeviceStateChanged(gcnew Device(this, gcnew String(pwstrDeviceId)), gcnew DeviceStateChangedEventArgs((DeviceState)dwNewState));
		}

		void Controller::FirePropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) {
			//this->OnPropertyValueChanged(gcnew Device(this, gcnew String(pwstrDeviceId)), nullptr);
		}

		array<Device^>^ Controller::GetAudioDevices(DeviceType type, DeviceState stateMask) {
			CComPtr<IMMDeviceEnumerator> pEnumerator = nullptr;

			HRESULT hr = pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			CComPtr<IMMDeviceCollection> pCollection = nullptr;

			hr = pEnumerator->EnumAudioEndpoints((EDataFlow)type, (DWORD)stateMask, &pCollection);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			UINT count;

			hr = pCollection->GetCount(&count);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			array<Device^>^ endpoints = gcnew array<Device^>(count);

			for (UINT i = 0; i < count; i++) {
				CComPtr<IMMDevice> pDevice = nullptr;

				hr = pCollection->Item(i, &pDevice);

				if (FAILED(hr)) {
					throw gcnew ApplicationException(ConvertHrToString(hr));
				}

				LPWSTR pwszId = nullptr;

				hr = pDevice->GetId(&pwszId);

				if (FAILED(hr)) {
					throw gcnew ApplicationException(ConvertHrToString(hr));
				}

				String^ id = gcnew String(pwszId);

				CoTaskMemFree(pwszId);

				endpoints[i] = gcnew Device(this, id);
			}

			return endpoints;
		}

		Device^ Controller::GetAudioDevice(String^ id) {
			return gcnew Device(this, id);
		}

		Device^ Controller::GetDefaultAudioDevice(DeviceType type, DeviceRole role) {
			CComPtr<IMMDeviceEnumerator> pEnumerator = nullptr;

			HRESULT hr = pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			CComPtr<IMMDevice> pDevice = nullptr;

			hr = pEnumerator->GetDefaultAudioEndpoint((EDataFlow)type, (ERole)role, &pDevice);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			LPWSTR pwszId = nullptr;

			hr = pDevice->GetId(&pwszId);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			String^ id = gcnew String(pwszId);

			CoTaskMemFree(pwszId);

			return gcnew Device(this, id);
		}

		void Controller::SetDefaultAudioDevice(Device^ endpoint, DeviceRole role) {
			CComPtr<IPolicyConfigVista> pPolicyConfig = nullptr;

			HRESULT hr = pPolicyConfig.CoCreateInstance(__uuidof(CPolicyConfigVistaClient));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			IntPtr hId = Marshal::StringToHGlobalUni(endpoint->Id);
			LPWSTR pwszId = (LPWSTR)hId.ToPointer();

			hr = pPolicyConfig->SetDefaultEndpoint(pwszId, (ERole)role);

			Marshal::FreeHGlobal(hId);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			// Delay for the device to actually switch over
			Sleep(300);
		}
	}
}
