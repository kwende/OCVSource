#include "stdafx.h"
#include "OCVStream.h"
#include "Mfapi.h"
#include <iostream>

using namespace std; 

OCVStream::OCVStream(OCVSource* pOcvSource, IMFStreamDescriptor *pSD)
{
    ::InitializeCriticalSection(&m_CriticalSection); 
    MFCreateEventQueue(&m_pEventQueue);

    m_pSource = pOcvSource; 
    m_pSource->AddRef(); 

    m_pDescriptor = pSD; 
    m_pDescriptor->AddRef(); 

    position = 0; 
}


OCVStream::~OCVStream()
{
    m_pSource->Release(); 
    m_pDescriptor->Release(); 
}


// IUnknown
STDMETHODIMP OCVStream::QueryInterface(REFIID iid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(OCVStream, IMFMediaEventGenerator),
        QITABENT(OCVStream, IMFMediaStream),
        { 0 }
    };
    return QISearch(this, qit, iid, ppv);
}
STDMETHODIMP_(ULONG) OCVStream::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}
STDMETHODIMP_(ULONG) OCVStream::Release()
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
STDMETHODIMP OCVStream::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState)
{
    ATLTRACE2("BENR: OCVStream::BeginGetEvent");

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CriticalSection);

    hr = m_pEventQueue->BeginGetEvent(pCallback, punkState);

    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}
STDMETHODIMP OCVStream::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent)
{
    ATLTRACE2("BENR: OCVStream::EndGetEvent");

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CriticalSection);

    hr = m_pEventQueue->EndGetEvent(pResult, ppEvent);

    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}
STDMETHODIMP OCVStream::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    ATLTRACE2("BENR: OCVStream::GetEvent");

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
STDMETHODIMP OCVStream::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue)
{
    ATLTRACE2("BENR: OCVStream::QueueEvent");

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

// IMFMediaStream
STDMETHODIMP OCVStream::GetMediaSource(IMFMediaSource** ppMediaSource)
{
    ATLTRACE2("BENR: OCVStream::GetMediaSource");

    EnterCriticalSection(&m_CriticalSection); 

    HRESULT hr = m_pSource->QueryInterface(IID_PPV_ARGS(ppMediaSource)); 

    LeaveCriticalSection(&m_CriticalSection); 
    return hr;
}
STDMETHODIMP OCVStream::GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor)
{
    ATLTRACE2("BENR: OCVStream::GetStreamDescriptor");

    EnterCriticalSection(&m_CriticalSection); 

    HRESULT hr = m_pDescriptor->QueryInterface(IID_PPV_ARGS(ppStreamDescriptor)); 

    LeaveCriticalSection(&m_CriticalSection); 
    return hr;
}
STDMETHODIMP OCVStream::RequestSample(IUnknown* pToken)
{
    ATLTRACE2("BENR: OCVStream::RequestSample");

    ::EnterCriticalSection(&m_CriticalSection); 

    IMFSample *pSample = NULL;

    IMFMediaBuffer* pBuffer; 
    HRESULT hr = MFCreateMemoryBuffer(3 * 8 * 512 * 424, &pBuffer);

    if (SUCCEEDED(hr))
    {
        hr = MFCreateSample(&pSample);

        hr = pSample->AddBuffer(pBuffer);
        pSample->AddRef();

        pSample->SetSampleTime(position);

        position += 10000000;

        if (pToken)
        {
            pSample->SetUnknown(MFSampleExtension_Token, pToken);
        }

        pSample->SetSampleDuration(10000000);

        hr = QueueEventWithIUnknown(this, MEMediaSample, hr, pSample);
    }
    else
    {
        cout << "Fart" << endl; 
    }

    LeaveCriticalSection(&m_CriticalSection); 

    return hr;
}