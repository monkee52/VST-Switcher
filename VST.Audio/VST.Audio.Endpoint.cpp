#include "stdafx.h"

#include "VST.Audio.h"

using namespace System::Runtime::InteropServices;

namespace VST {
	namespace Audio {
		Endpoint::Endpoint(Controller^ utils, String^ id) {
			this->Utils = utils;
			this->Id = id;
		}

		String^ Endpoint::Id::get() {
			return this->_id;
		}

		void Endpoint::Id::set(String^ id) {
			this->_id = id;
		}

		String^ Endpoint::Name::get() {
			return GetPropertyCommon(this->Id, PKEY_DeviceInterface_FriendlyName);
		}

		String^ Endpoint::FriendlyName::get() {
			return GetPropertyCommon(this->Id, PKEY_Device_FriendlyName);
		}

		String^ Endpoint::Description::get() {
			return GetPropertyCommon(this->Id, PKEY_Device_DeviceDesc);
		}

		bool Endpoint::IsDefault::get() {
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

			String^ defaultId = gcnew String(wszId);

			CoTaskMemFree(wszId);

			return this->Id == defaultId;
		}

		EndpointState Endpoint::State::get() {
			CComPtr<IMMDeviceEnumerator> pEnumerator = nullptr;

			HRESULT hr = pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			CComPtr<IMMDevice> pEndpoint = nullptr;

			IntPtr hId = Marshal::StringToHGlobalUni(this->Id);
			LPWSTR pwstrId = (LPWSTR)hId.ToPointer();

			hr = pEnumerator->GetDevice(pwstrId, &pEndpoint);

			Marshal::FreeHGlobal(hId);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			DWORD dwState;

			hr = pEndpoint->GetState(&dwState);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			return (EndpointState)dwState;
		}

		float Endpoint::Volume::get() {
			float volume;

			VolumeCommon(this->Id, false, &volume, nullptr);

			return volume;
		}

		void Endpoint::Volume::set(float volume) {
			volume = (volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume));

			VolumeCommon(this->Id, true, &volume, nullptr);
		}

		bool Endpoint::Muted::get() {
			bool mute;

			VolumeCommon(this->Id, false, nullptr, &mute);

			return mute;
		}

		void Endpoint::Muted::set(bool mute) {
			VolumeCommon(this->Id, true, nullptr, &mute);
		}

		bool Endpoint::Equals(Object^ otherEndpoint) {
			if (otherEndpoint == nullptr) {
				return false;
			}

			Endpoint^ that = dynamic_cast<Endpoint^>(otherEndpoint);

			return this->Id == that->Id;
		}

		bool Endpoint::Equals(Endpoint^ that) {
			if (that == nullptr) {
				return false;
			}

			return this->Id == that->Id;
		}

		bool Endpoint::operator!= (Endpoint^ a, Endpoint^ b) {
			return !(a == b);
		}

		bool Endpoint::operator== (Endpoint^ a, Endpoint^ b) {
			if (Object::ReferenceEquals(a, b)) {
				return true;
			}

			if ((dynamic_cast<Object^>(a) == nullptr) || (dynamic_cast<Object^>(b) == nullptr)) {
				return false;
			}

			return a->Id == b->Id;
		}

		int Endpoint::GetHashCode() {
			return this->ToString()->GetHashCode();
		}

		String^ Endpoint::ToString() {
			return String::Format(gcnew String(_T("<#Endpoint({0})>")), this->Id);
		}
	}
}
