// OCVSource.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "OCVSource.h"
#include <Dvdmedia.h>
#include <Dshow.h>
#include <atlbase.h>

OCVSource::OCVSource()
{
    DllAddRef();
}

OCVSource::~OCVSource()
{
    DllRelease();
}

HRESULT OCVSource::CreateInstance(REFIID iid, void **ppSource)
{
    ATLTRACE2("BENR: OCVSource::CreateInstance");

    if (ppSource == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    OCVSource* pSource = new (std::nothrow) OCVSource();
    pSource->m_pStream = NULL;

    hr = MFCreateEventQueue(&pSource->m_pEventQueue);

    if (SUCCEEDED(hr))
    {
        InitializeCriticalSection(&pSource->m_CriticalSection);
        if (pSource == NULL)
        {
            return E_OUTOFMEMORY;
        }

        pSource->AddRef();

        if (SUCCEEDED(hr))
        {
            hr = pSource->QueryInterface(iid, ppSource);
        }
    }

    pSource->Release();

    return hr;
}

// IUnknown
STDMETHODIMP OCVSource::QueryInterface(REFIID iid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(OCVSource, IMFMediaEventGenerator),
        QITABENT(OCVSource, IMFMediaSource),
        { 0 }
    };
    return QISearch(this, qit, iid, ppv);
}

ULONG OCVSource::AddRef()
{
    return ::InterlockedIncrement(&m_nRefCount);
}

ULONG OCVSource::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}


// IMFMediaEventGenerator
STDMETHODIMP OCVSource::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState)
{
    ATLTRACE2("BENR: OCVSource::BeginGetEvent");

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CriticalSection);

    hr = m_pEventQueue->BeginGetEvent(pCallback, punkState);

    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

STDMETHODIMP OCVSource::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent)
{
    ATLTRACE2("BENR: OCVSource::EndGetEvent");

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CriticalSection);

    hr = m_pEventQueue->EndGetEvent(pResult, ppEvent);

    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

STDMETHODIMP OCVSource::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    ATLTRACE2("BENR: OCVSource::GetEvent");

    HRESULT hr = S_OK;

    IMFMediaEventQueue *pQueue = NULL;

    EnterCriticalSection(&m_CriticalSection);

    // Check shutdown
    pQueue = m_pEventQueue;
    pQueue->AddRef();

    LeaveCriticalSection(&m_CriticalSection);

    hr = pQueue->GetEvent(dwFlags, ppEvent);

    pQueue->Release();

    return hr;
}

STDMETHODIMP OCVSource::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue)
{
    ATLTRACE2("BENR: OCVSource::QueueEvent");

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CriticalSection);

    //hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
    }

    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}


// IMFMediaSource
STDMETHODIMP OCVSource::CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor)
{
    ATLTRACE2("BENR: OCVSource::CreatePresentationDescriptor");

    IMFMediaType *pMediaType = NULL;
    IMFStreamDescriptor* pStreamDescriptor = NULL;

    HRESULT hr = ::MFCreateMediaType(&pMediaType);

    if (SUCCEEDED(hr))
    {
        hr = pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        if (SUCCEEDED(hr))
        {
            hr = pMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB24);
            if (SUCCEEDED(hr))
            {
                hr = pMediaType->SetUINT32(MF_MT_AVG_BITRATE, 800000);
                MFSetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, 512, 424);
                MFSetAttributeRatio(pMediaType, MF_MT_FRAME_RATE, 30, 1);
                MFSetAttributeRatio(pMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);

                hr = MFCreateStreamDescriptor(0, 1, &pMediaType, &pStreamDescriptor);

                IMFMediaTypeHandler* pMediaTypeHandler;
                pStreamDescriptor->GetMediaTypeHandler(&pMediaTypeHandler);

                hr = pMediaTypeHandler->SetCurrentMediaType(pMediaType);

                IMFPresentationDescriptor* localDescriptor;
                hr = MFCreatePresentationDescriptor(1, &pStreamDescriptor, &localDescriptor);

                if (SUCCEEDED(hr))
                {
                    hr = localDescriptor->SelectStream(0);

                    if (SUCCEEDED(hr))
                    {
                        hr = localDescriptor->Clone(ppPresentationDescriptor); 
                    }
                }
            }
        }
    }

    pMediaType->Release(); 
    pStreamDescriptor->Release(); 

    return hr;
}

STDMETHODIMP OCVSource::GetCharacteristics(DWORD* pdwCharacteristics)
{
    ATLTRACE2("BENR: OCVSource::GetCharacteristics");

    if (pdwCharacteristics == NULL)
    {
        *pdwCharacteristics = MFMEDIASOURCE_CAN_PAUSE | MFMEDIASOURCE_CAN_SEEK;
    }

    return S_OK;
}

STDMETHODIMP OCVSource::Pause()
{
    ATLTRACE2("BENR: OCVSource::Pause");

    return S_OK;
}

STDMETHODIMP OCVSource::Shutdown()
{
    ATLTRACE2("BENR: OCVSource::Shutdown");
    return S_OK;
}

STDMETHODIMP OCVSource::Start(
    IMFPresentationDescriptor* pPresentationDescriptor,
    const GUID* pguidTimeFormat,
    const PROPVARIANT* pvarStartPosition
    )
{
    ATLTRACE2("BENR: OCVSource::Start");

    IMFStreamDescriptor* pStreamDescriptor = NULL;
    BOOL selected;
    HRESULT hr = pPresentationDescriptor->GetStreamDescriptorByIndex(0, &selected, &pStreamDescriptor);

    PROPVARIANT var;
    PropVariantInit(&var);
    IMFMediaEvent *pEvent = NULL;

    if (SUCCEEDED(hr))
    {
        if (m_pStream == NULL)
        {
            // create stream. 
            m_pStream = new OCVStream(this, pStreamDescriptor);

            QueueEventWithIUnknown(this, MENewStream, S_OK, (IMFMediaSource*)m_pStream);
        }
        else
        {
            hr = QueueEventWithIUnknown(this, MEUpdatedStream, S_OK, (IMFMediaSource*)m_pStream);
        }

        hr = MFCreateMediaEvent(MESourceStarted, GUID_NULL, hr, &var, &pEvent);


        hr = m_pEventQueue->QueueEvent(pEvent);
        hr = m_pStream->QueueEvent(MEStreamStarted, GUID_NULL, hr, &var);
    }

    return S_OK;
}

STDMETHODIMP OCVSource::Stop()
{
    ATLTRACE2("BENR: OCVSource::Stop");
    return S_OK;
}


// Other methods
HRESULT OCVSource::Open(IMFByteStream *pStream)
{
    ATLTRACE2("BENR: OCVSource::Open");
    return S_OK;
}

HRESULT OCVSource::QueueEventWithIUnknown(
    IMFMediaEventGenerator *pMEG,
    MediaEventType meType,
    HRESULT hrStatus,
    IUnknown *pUnk)
{
    ATLTRACE2("BENR: OCVSource::QueueEventWithIUnknown");
    // Create the PROPVARIANT to hold the IUnknown value.
    PROPVARIANT var;
    var.vt = VT_UNKNOWN;
    var.punkVal = pUnk;
    pUnk->AddRef();

    // Queue the event.
    HRESULT hr = pMEG->QueueEvent(meType, GUID_NULL, hrStatus, &var);

    // Clear the PROPVARIANT.
    PropVariantClear(&var);

    return hr;
}
