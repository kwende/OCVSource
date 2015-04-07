#pragma once
#include <Windows.h>
#include <Mfidl.h>
#include <Mfapi.h>

void DllAddRef();
void DllRelease();

class OCVByteStreamHandler : public IMFByteStreamHandler
{
public:

    static HRESULT CreateInstance(REFIID iid, void **ppMEG);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFByteStreamHandler

    STDMETHODIMP BeginCreateObject(
        /* [in] */ IMFByteStream *pByteStream,
        /* [in] */ LPCWSTR pwszURL,
        /* [in] */ DWORD dwFlags,
        /* [in] */ IPropertyStore *pProps,
        /* [out] */ IUnknown **ppIUnknownCancelCookie,
        /* [in] */ IMFAsyncCallback *pCallback,
        /* [in] */ IUnknown *punkState);

    STDMETHODIMP EndCreateObject(
        /* [in] */ IMFAsyncResult *pResult,
        /* [out] */ MF_OBJECT_TYPE *pObjectType,
        /* [out] */ IUnknown **ppObject);

    STDMETHODIMP CancelObjectCreation(IUnknown *pIUnknownCancelCookie);
    STDMETHODIMP GetMaxNumberOfBytesRequiredForResolution(QWORD* pqwBytes);

private:
    OCVByteStreamHandler();
    ~OCVByteStreamHandler();

    long m_nRefCount; // reference count
};

