#include "stdafx.h"
#include "OCVByteStreamHandler.h"
#include <assert.h>
#include "OCVSource.h"

OCVByteStreamHandler::OCVByteStreamHandler() : m_nRefCount(0)
{
    DllAddRef();
}


OCVByteStreamHandler::~OCVByteStreamHandler()
{
    assert(m_nRefCount == 0);

    DllRelease();
}

HRESULT OCVByteStreamHandler::CreateInstance(REFIID iid, void **ppMEG)
{
    if (ppMEG == NULL)
    {
        return E_POINTER;
    }

    // Create the object.
    HRESULT hr = S_OK;
    OCVByteStreamHandler *pHandler = new (std::nothrow) OCVByteStreamHandler();
    if (pHandler == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Query for the requested interface.
    if (SUCCEEDED(hr))
    {
        hr = pHandler->QueryInterface(iid, ppMEG);
    }

    if (FAILED(hr))
    {
        delete pHandler;
    }
    return hr;
}

// IUnknown
STDMETHODIMP OCVByteStreamHandler::QueryInterface(REFIID iid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(OCVByteStreamHandler, IMFByteStreamHandler),
        { 0 }
    };
    return QISearch(this, qit, iid, ppv);

}
STDMETHODIMP_(ULONG) OCVByteStreamHandler::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}
STDMETHODIMP_(ULONG) OCVByteStreamHandler::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

// IMFByteStreamHandler
STDMETHODIMP OCVByteStreamHandler::BeginCreateObject(
    /* [in] */ IMFByteStream *pByteStream,
    /* [in] */ LPCWSTR pwszURL,
    /* [in] */ DWORD dwFlags,
    /* [in] */ IPropertyStore *pProps,
    /* [out] */ IUnknown **ppIUnknownCancelCookie,
    /* [in] */ IMFAsyncCallback *pCallback,
    /* [in] */ IUnknown *punkState)
{
    if ((pByteStream == NULL) || (pwszURL == NULL) || (pCallback == NULL))
    {
        return E_INVALIDARG;
    }

    if (ppIUnknownCancelCookie)
    {
        *ppIUnknownCancelCookie = NULL; // We don't return a cancellation cookie.
    }

    //CComPtr<IMFMediaSource> pSource = NULL; 
    //CComPtr<IMFAsyncResult> pResult = NULL; 

    IMFMediaSource *pSource = NULL;
    IMFAsyncResult *pResult = NULL;

    HRESULT hr = OCVSource::CreateInstance(IID_IMFMediaSource, (LPVOID*)&pSource); 

    if (SUCCEEDED(hr))
    {
        hr = ((OCVSource*)(IMFMediaSource*)(pSource))->Open(pByteStream); 

        if (SUCCEEDED(hr))
        {
            // wrap in the async object. 
            hr = ::MFCreateAsyncResult(pSource, pCallback, punkState, &pResult); 
            if (SUCCEEDED(hr))
            {
                // just call the callback now. 
                MFInvokeCallback(pResult); 
            }
        }
    }

    pSource->Release(); 
    pResult->Release(); 

    return hr; 
}

STDMETHODIMP OCVByteStreamHandler::EndCreateObject(
    /* [in] */ IMFAsyncResult *pResult,
    /* [out] */ MF_OBJECT_TYPE *pObjectType,
    /* [out] */ IUnknown **ppObject)
{
    HRESULT hr = S_OK;

    if ((pResult == NULL) || (ppObject == NULL) || (ppObject == NULL))
    {
        return E_INVALIDARG;
    }

    //CComPtr<IMFMediaSource> pSource = NULL;
    //CComPtr<IUnknown> pUnk = NULL;
    IMFMediaSource* pSource = NULL; 
    IUnknown* pUnk = NULL; 

    hr = pResult->GetObject(&pUnk);

    if (SUCCEEDED(hr))
    {
        // Minimal sanity check - is it really a media source?
        hr = pUnk->QueryInterface(IID_PPV_ARGS(&pSource));
    }

    if (SUCCEEDED(hr))
    {
        *ppObject = pUnk;
        (*ppObject)->AddRef();
        *pObjectType = MF_OBJECT_MEDIASOURCE;
    }

    pSource->Release(); 
    pUnk->Release(); 

    return hr; 
}

STDMETHODIMP OCVByteStreamHandler::CancelObjectCreation(IUnknown *pIUnknownCancelCookie)
{
    return E_NOTIMPL;
}

STDMETHODIMP OCVByteStreamHandler::GetMaxNumberOfBytesRequiredForResolution(QWORD* pqwBytes)
{
    if (pqwBytes == NULL)
    {
        return E_INVALIDARG;
    }
    else
    {
        *pqwBytes = 1024; 
    }
}