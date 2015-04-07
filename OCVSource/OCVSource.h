#pragma once

#include <Windows.h>
#include <Mfidl.h>
#include <Mfapi.h>
#include "OCVStream.h"

void DllAddRef();
void DllRelease();

class OCVSource : public IMFMediaSource, public IMFMediaEventGenerator
{
    friend class OCVStream; 

public:
    static HRESULT CreateInstance(REFIID iid, void **ppSource);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFMediaEventGenerator
    STDMETHODIMP BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState);
    STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent);
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent);
    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue);

    // IMFMediaSource
    STDMETHODIMP CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor);
    STDMETHODIMP GetCharacteristics(DWORD* pdwCharacteristics);
    STDMETHODIMP Pause();
    STDMETHODIMP Shutdown();
    STDMETHODIMP Start(
        IMFPresentationDescriptor* pPresentationDescriptor,
        const GUID* pguidTimeFormat,
        const PROPVARIANT* pvarStartPosition
        );
    STDMETHODIMP Stop();

    // Other methods
    HRESULT Open(IMFByteStream *pStream);

private: 
    long     m_nRefCount;
    IMFMediaEventQueue  *m_pEventQueue;
    CRITICAL_SECTION m_CriticalSection; 
    OCVStream * m_pStream; 
    OCVSource(); 
    ~OCVSource(); 

    HRESULT QueueEventWithIUnknown(
        IMFMediaEventGenerator *pMEG,
        MediaEventType meType,
        HRESULT hrStatus,
        IUnknown *pUnk); 
};