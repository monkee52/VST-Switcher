#include "stdafx.h"

#include "VST.Audio.h"

namespace VST {
	namespace Audio {
		DefaultDeviceChangedEventArgs::DefaultDeviceChangedEventArgs(DeviceType type, DeviceRole role) {
			this->_type = type;
			this->_role = role;
		}

		DeviceType DefaultDeviceChangedEventArgs::Type::get() {
			return this->_type;
		}

		DeviceRole DefaultDeviceChangedEventArgs::Role::get() {
			return this->_role;
		}
	}
}
