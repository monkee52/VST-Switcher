#include "stdafx.h"

#include "VST.Audio.h"

using namespace System::Runtime::InteropServices;

namespace VST {
	namespace Audio {
		Device::Device(Controller^ utils, String^ id) {
			this->Utils = utils;
			this->Id = id;
		}

		String^ Device::Id::get() {
			return this->_id;
		}

		void Device::Id::set(String^ id) {
			this->_id = id;
		}

		String^ Device::Name::get() {
			return GetPropertyCommon(this->Id, PKEY_DeviceInterface_FriendlyName);
		}

		String^ Device::FriendlyName::get() {
			return GetPropertyCommon(this->Id, PKEY_Device_FriendlyName);
		}

		String^ Device::Description::get() {
			return GetPropertyCommon(this->Id, PKEY_Device_DeviceDesc);
		}

		bool Device::IsDefault(DeviceType type, DeviceRole role) {
			return this->Utils->GetDefaultAudioDevice(type, role) == this;
		}

		DeviceState Device::State::get() {
			CComPtr<IMMDeviceEnumerator> pEnumerator = nullptr;

			HRESULT hr = pEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			CComPtr<IMMDevice> pDevice = nullptr;

			IntPtr hId = Marshal::StringToHGlobalUni(this->Id);
			LPWSTR pwstrId = (LPWSTR)hId.ToPointer();

			hr = pEnumerator->GetDevice(pwstrId, &pDevice);

			Marshal::FreeHGlobal(hId);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			DWORD dwState;

			hr = pDevice->GetState(&dwState);

			if (FAILED(hr)) {
				throw gcnew ApplicationException(ConvertHrToString(hr));
			}

			return (DeviceState)dwState;
		}

		Single Device::Volume::get() {
			float volume;

			VolumeCommon(this->Id, false, &volume, nullptr);

			return volume;
		}

		void Device::Volume::set(float volume) {
			volume = (volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume));

			VolumeCommon(this->Id, true, &volume, nullptr);
		}

		bool Device::Muted::get() {
			bool mute;

			VolumeCommon(this->Id, false, nullptr, &mute);

			return mute;
		}

		void Device::Muted::set(bool mute) {
			VolumeCommon(this->Id, true, nullptr, &mute);
		}

		bool Device::Equals(Object^ otherDevice) {
			if (otherDevice == nullptr) {
				return false;
			}

			Device^ that = dynamic_cast<Device^>(otherDevice);

			return this->Id == that->Id;
		}

		bool Device::Equals(Device^ that) {
			if (that == nullptr) {
				return false;
			}

			return this->Id == that->Id;
		}

		bool Device::operator!= (Device^ a, Device^ b) {
			return !(a == b);
		}

		bool Device::operator== (Device^ a, Device^ b) {
			if (Object::ReferenceEquals(a, b)) {
				return true;
			}

			if ((dynamic_cast<Object^>(a) == nullptr) || (dynamic_cast<Object^>(b) == nullptr)) {
				return false;
			}

			return a->Id == b->Id;
		}

		int Device::GetHashCode() {
			return this->ToString()->GetHashCode();
		}

		String^ Device::ToString() {
			return String::Format(gcnew String(_T("<#Device({0})>")), this->Id);
		}
	}
}
