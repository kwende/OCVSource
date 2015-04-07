#pragma once

#include <Windows.h>
#include <Mfidl.h>
#include "OCVSource.h"

class OCVStream : public IMFMediaStream, public IMFMediaEventGenerator
{
    friend class OCVSource; 

public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFMediaEventGenerator
    STDMETHODIMP BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState);
    STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent);
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent);
    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue);

    // IMFMediaStream
    STDMETHODIMP GetMediaSource(IMFMediaSource** ppMediaSource);
    STDMETHODIMP GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor);
    STDMETHODIMP RequestSample(IUnknown* pToken);

private: 
    OCVStream(OCVSource* pOcvSource, IMFStreamDescriptor *pSD);
    ~OCVStream();

    IMFMediaEventQueue *m_pEventQueue;
    CRITICAL_SECTION m_CriticalSection;
    ULONG m_nRefCount; 
    OCVSource* m_pSource; 
    IMFStreamDescriptor* m_pDescriptor; 

    HRESULT QueueEventWithIUnknown(
        IMFMediaEventGenerator *pMEG,
        MediaEventType meType,
        HRESULT hrStatus,
        IUnknown *pUnk)
    {

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

    LONGLONG position; 
};

