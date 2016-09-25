#include "stdafx.h"

#include "VST.Audio.h"

namespace VST {
	namespace Audio {
		CMMNotificationClient::CMMNotificationClient(void* pController) {
			this->_cRef = 1;
			
			IntPtr mpController(pController);
			GCHandle hController = GCHandle::FromIntPtr(mpController);

			this->hController = hController;
		}

		CMMNotificationClient::~CMMNotificationClient() {
			this->hController.Free();
		}

		ULONG CMMNotificationClient::AddRef() {
			return InterlockedIncrement(&this->_cRef);
		}

		ULONG CMMNotificationClient::Release() {
			ULONG ulRef = InterlockedDecrement(&this->_cRef);

			if (ulRef == 0) {
				delete this;
			}

			return ulRef;
		}

		HRESULT CMMNotificationClient::QueryInterface(REFIID riid, VOID** ppvInterface) {
			if (riid == IID_IUnknown) {
				AddRef();

				*ppvInterface = (IUnknown*)this;
			} else if (riid == __uuidof(IMMNotificationClient)) {
				AddRef();

				*ppvInterface = (IMMNotificationClient*)this;
			} else {
				*ppvInterface = nullptr;

				return E_NOINTERFACE;
			}

			return S_OK;
		}

		HRESULT CMMNotificationClient::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDevice) {
			((Controller^)this->hController.Target)->FireDefaultDeviceChanged(flow, role, pwstrDefaultDevice);

			return S_OK;
		}

		HRESULT CMMNotificationClient::OnDeviceAdded(LPCWSTR pwstrDeviceId) {
			((Controller^)this->hController.Target)->FireDeviceAdded(pwstrDeviceId);

			return S_OK;
		}

		HRESULT CMMNotificationClient::OnDeviceRemoved(LPCWSTR pwstrDeviceId) {
			((Controller^)this->hController.Target)->FireDeviceRemoved(pwstrDeviceId);

			return S_OK;
		}

		HRESULT CMMNotificationClient::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) {
			((Controller^)this->hController.Target)->FireDeviceStateChanged(pwstrDeviceId, dwNewState);

			return S_OK;
		}

		HRESULT CMMNotificationClient::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) {
			((Controller^)this->hController.Target)->FirePropertyValueChanged(pwstrDeviceId, key);

			return S_OK;
		}
	}
}
