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
			CComPtr<IMMDeviceEnumerator> pEnumerator = nullptr;

			HRESULT hr = pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			CComPtr<IMMDevice> pEndpoint = nullptr;

			IntPtr hId = Marshal::StringToHGlobalUni(this->Id);
			LPWSTR szId = (LPWSTR)hId.ToPointer();

			hr = pEnumerator->GetDevice(szId, &pEndpoint);

			Marshal::FreeHGlobal(hId);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			if (pEndpoint == nullptr) {
				throw gcnew ApplicationException(String::Format(gcnew String(_T("Unable to find device {0}")), this->Id));
			}

			CComPtr<IPropertyStore> pProps = nullptr;

			hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			PROPVARIANT varName;

			PropVariantInit(&varName);

			hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);

			if (FAILED(hr)) {
				PropVariantClear(&varName);

				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			String^ name = gcnew String(varName.pwszVal);

			PropVariantClear(&varName);

			return name;
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

			return bool(this->Id == defaultId);
		}

		float Endpoint::GetVolume() {
			float volume;

			VolumeCommon(this->Id, false, &volume, nullptr);

			return volume;
		}

		void Endpoint::SetVolume(float volume) {
			volume = (volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume));

			VolumeCommon(this->Id, true, &volume, nullptr);
		}

		bool Endpoint::GetMute() {
			bool mute;

			VolumeCommon(this->Id, false, nullptr, &mute);

			return bool(mute);
		}

		void Endpoint::SetMute(bool mute) {
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
