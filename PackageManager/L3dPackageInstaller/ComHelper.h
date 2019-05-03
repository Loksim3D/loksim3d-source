#pragma once

#include <Windows.h>
#include <OCIdl.h>

#ifdef _ATL_DISABLE_NOINLINE
#define ATL_NOINLINE
#else
#define ATL_NOINLINE __declspec( noinline )
#endif


#define ATLAPI __declspec(nothrow) HRESULT __stdcall
#define ATLAPI_(x) __declspec(nothrow) x __stdcall
#define ATLAPIINL ATLAPI
#define ATLAPIINL_(x) ATLAPI_(x)
#define ATLINLINE inline

#ifndef _CrtDbgBreak
	#define _CrtDbgBreak() __debugbreak()
#endif 

_CRTIMP int __cdecl _CrtDbgReport(
        _In_ int _ReportType,
        _In_opt_z_ const char * _Filename,
        _In_ int _Linenumber,
        _In_opt_z_ const char * _ModuleName,
        _In_opt_z_ const char * _Format,
        ...);

_CRTIMP int __cdecl _CrtDbgReportW(
        _In_ int _ReportType,
        _In_opt_z_ const wchar_t * _Filename,
        _In_ int _LineNumber,
        _In_opt_z_ const wchar_t * _ModuleName,
        _In_opt_z_ const wchar_t * _Format,
        ...);

#define _CRT_WARN           0
#define _CRT_ERROR          1
#define _CRT_ASSERT         2
#define _CRT_ERRCNT         3

/* Asserts */
/* We use !! below to ensure that any overloaded operators used to evaluate expr do not end up at operator || */

#ifndef _ASSERT_EXPR
	#define _ASSERT_EXPR(expr, msg) \
			(void) ((!!(expr)) || \
					(1 != _CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, msg)) || \
					(_CrtDbgBreak(), 0))
#endif

#ifndef _ASSERT
#define _ASSERT(expr)   _ASSERT_EXPR((expr), NULL)
#endif

#ifndef _ASSERTE
#define _ASSERTE(expr)  _ASSERT_EXPR((expr), _CRT_WIDE(#expr))
#endif

#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif // ATLASSERT

/////////////////////////////////////////////////////////////////////////////
// Error to HRESULT helpers



/////////////////////////////////////////////////////////////////////////////
// Smart Pointer helpers

ATLAPI_(IUnknown*) AtlComPtrAssign(
	_Inout_opt_ _Deref_pre_maybenull_ _Deref_post_maybenull_ IUnknown** pp, 
	_In_opt_ IUnknown* lp);

ATLAPI_(IUnknown*) AtlComQIPtrAssign(
	_Inout_opt_ _Deref_pre_maybenull_ _Deref_post_maybenull_ IUnknown** pp, 
	_In_opt_ IUnknown* lp, 
	_In_ REFIID riid);



/////////////////////////////////////////////////////////////////////////////
// COM Smart pointers

template <class T>
class _NoAddRefReleaseOnCComPtr : 
	public T
{
	private:
		STDMETHOD_(ULONG, AddRef)()=0;
		STDMETHOD_(ULONG, Release)()=0;
};

_Check_return_ inline HRESULT AtlSetChildSite(
	_Inout_ IUnknown* punkChild, 
	_Inout_opt_ IUnknown* punkParent)
{
	if (punkChild == NULL)
		return E_POINTER;

	HRESULT hr;
	IObjectWithSite* pChildSite = NULL;
	hr = punkChild->QueryInterface(__uuidof(IObjectWithSite), (void**)&pChildSite);
	if (SUCCEEDED(hr) && pChildSite != NULL)
	{
		hr = pChildSite->SetSite(punkParent);
		pChildSite->Release();
	}
	return hr;
}


//CComPtrBase provides the basis for all other smart pointers
//The other smartpointers add their own constructors and operators
template <class T>
class CComPtrBase
{
protected:
	CComPtrBase() throw()
	{
		p = NULL;
	}
	CComPtrBase(_Inout_opt_ T* lp) throw()
	{
		p = lp;
		if (p != NULL)
			p->AddRef();
	}
public:
	typedef T _PtrClass;
	~CComPtrBase() throw()
	{
		if (p)
			p->Release();
	}
	operator T*() const throw()
	{
		return p;
	}
	T& operator*() const
	{
		ATLENSURE(p!=NULL);
		return *p;
	}
	//The assert on operator& usually indicates a bug.  If this is really
	//what is needed, however, take the address of the p member explicitly.
	T** operator&() throw()
	{
		ATLASSERT(p==NULL);
		return &p;
	}
	_NoAddRefReleaseOnCComPtr<T>* operator->() const throw()
	{
		ATLASSERT(p!=NULL);
		return (_NoAddRefReleaseOnCComPtr<T>*)p;
	}
	bool operator!() const throw()
	{	
		return (p == NULL);
	}
	bool operator<(_In_opt_ T* pT) const throw()
	{
		return p < pT;
	}
	bool operator!=(_In_opt_ T* pT) const
	{
		return !operator==(pT);
	}
	bool operator==(_In_opt_ T* pT) const throw()
	{
		return p == pT;
	}

	// Release the interface and set to NULL
	void Release() throw()
	{
		T* pTemp = p;
		if (pTemp)
		{
			p = NULL;
			pTemp->Release();
		}
	}
	// Compare two objects for equivalence
	bool IsEqualObject(_Inout_opt_ IUnknown* pOther) throw()
	{
		if (p == NULL && pOther == NULL)
			return true;	// They are both NULL objects

		if (p == NULL || pOther == NULL)
			return false;	// One is NULL the other is not

		CComPtr<IUnknown> punk1;
		CComPtr<IUnknown> punk2;
		p->QueryInterface(__uuidof(IUnknown), (void**)&punk1);
		pOther->QueryInterface(__uuidof(IUnknown), (void**)&punk2);
		return punk1 == punk2;
	}
	// Attach to an existing interface (does not AddRef)
	void Attach(_In_opt_ T* p2) throw()
	{
		if (p)
			p->Release();
		p = p2;
	}
	// Detach the interface (does not Release)
	T* Detach() throw()
	{
		T* pt = p;
		p = NULL;
		return pt;
	}
	_Check_return_ HRESULT CopyTo(_Deref_out_opt_ T** ppT) throw()
	{
		ATLASSERT(ppT != NULL);
		if (ppT == NULL)
			return E_POINTER;
		*ppT = p;
		if (p)
			p->AddRef();
		return S_OK;
	}
	_Check_return_ HRESULT SetSite(_Inout_opt_ IUnknown* punkParent) throw()
	{
		return AtlSetChildSite(p, punkParent);
	}
	_Check_return_ HRESULT Advise(
		_Inout_ IUnknown* pUnk, 
		_In_ const IID& iid, 
		_Out_ LPDWORD pdw) throw()
	{
		return AtlAdvise(p, pUnk, iid, pdw);
	}
	_Check_return_ HRESULT CoCreateInstance(
		_In_ REFCLSID rclsid, 
		_Inout_opt_ LPUNKNOWN pUnkOuter = NULL, 
		_In_ DWORD dwClsContext = CLSCTX_ALL) throw()
	{
		ATLASSERT(p == NULL);
		return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
	}
	_Check_return_ HRESULT CoCreateInstance(
		_In_z_ LPCOLESTR szProgID, 
		_Inout_opt_ LPUNKNOWN pUnkOuter = NULL, 
		_In_ DWORD dwClsContext = CLSCTX_ALL) throw()
	{
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(szProgID, &clsid);
		ATLASSERT(p == NULL);
		if (SUCCEEDED(hr))
			hr = ::CoCreateInstance(clsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
		return hr;
	}
	template <class Q>
	_Check_return_ HRESULT QueryInterface(_Deref_out_ Q** pp) const throw()
	{
		ATLASSERT(pp != NULL);
		return p->QueryInterface(__uuidof(Q), (void**)pp);
	}
	T* p;
};

template <class T>
class CComPtr : 
	public CComPtrBase<T>
{
public:
	CComPtr() throw()
	{
	}
	CComPtr(_Inout_opt_ T* lp) throw() :
		CComPtrBase<T>(lp)
	{
	}
	CComPtr(_Inout_ const CComPtr<T>& lp) throw() :
		CComPtrBase<T>(lp.p)
	{	
	}
	T* operator=(_Inout_opt_ T* lp) throw()
	{
        if(*this!=lp)
        {
    		return static_cast<T*>(AtlComPtrAssign((IUnknown**)&p, lp));
        }
        return *this;
	}
	template <typename Q>
	T* operator=(_Inout_ const CComPtr<Q>& lp) throw()
	{
        if( !IsEqualObject(lp) )
        {
    		return static_cast<T*>(AtlComQIPtrAssign((IUnknown**)&p, lp, __uuidof(T)));
        }
        return *this;
	}
	T* operator=(_Inout_ const CComPtr<T>& lp) throw()
	{
        if(*this!=lp)
        {
    		return static_cast<T*>(AtlComPtrAssign((IUnknown**)&p, lp));
        }
        return *this;
	}	
	CComPtr(_Inout_ CComPtr<T>&& lp) throw() :	
		CComPtrBase<T>()
	{	
		p = lp.p;		
		lp.p = NULL;
	}	
	T* operator=(_Inout_ CComPtr<T>&& lp) throw()
	{			
		if (*this != lp)
		{
			if (p != NULL)			
				p->Release();
			
			p = lp.p;
			lp.p = NULL;
		}
		return *this;		
	}
};

//specialization for IDispatch
template <>
class CComPtr<IDispatch> : 
	public CComPtrBase<IDispatch>
{
public:
	CComPtr() throw()
	{
	}
	CComPtr(_Inout_opt_ IDispatch* lp) throw() :
		CComPtrBase<IDispatch>(lp)
	{
	}
	CComPtr(_Inout_ const CComPtr<IDispatch>& lp) throw() :
		CComPtrBase<IDispatch>(lp.p)
	{
	}		
	IDispatch* operator=(_Inout_opt_ IDispatch* lp) throw()
	{
        if(*this!=lp)
        {
            return static_cast<IDispatch*>(AtlComPtrAssign((IUnknown**)&p, lp));
        }
        return *this;
	}
	IDispatch* operator=(_Inout_ const CComPtr<IDispatch>& lp) throw()
	{
        if(*this!=lp)
        {
    		return static_cast<IDispatch*>(AtlComPtrAssign((IUnknown**)&p, lp.p));
        }
        return *this;
	}	
	CComPtr(_Inout_ CComPtr<IDispatch>&& lp) throw() :	
		CComPtrBase<IDispatch>()
	{		
		p = lp.p;		
		lp.p = NULL;
	}
	IDispatch* operator=(_Inout_ CComPtr<IDispatch>&& lp) throw()
	{		
		if (*this != lp)
		{
			if (p != NULL)			
				p->Release();
			
			p = lp.p;
			lp.p = NULL;
		}		
		return *this;
	}	
// IDispatch specific stuff
	_Check_return_ HRESULT GetPropertyByName(
		_In_z_ LPCOLESTR lpsz, 
		_Out_ VARIANT* pVar) throw()
	{
		ATLASSERT(p);
		ATLASSERT(pVar);
		DISPID dwDispID;
		HRESULT hr = GetIDOfName(lpsz, &dwDispID);
		if (SUCCEEDED(hr))
			hr = GetProperty(dwDispID, pVar);
		return hr;
	}
	_Check_return_ HRESULT GetProperty(
		_In_ DISPID dwDispID, 
		_Out_ VARIANT* pVar) throw()
	{
		return GetProperty(p, dwDispID, pVar);
	}
	_Check_return_ HRESULT PutPropertyByName(
		_In_z_ LPCOLESTR lpsz, 
		_In_ VARIANT* pVar) throw()
	{
		ATLASSERT(p);
		ATLASSERT(pVar);
		DISPID dwDispID;
		HRESULT hr = GetIDOfName(lpsz, &dwDispID);
		if (SUCCEEDED(hr))
			hr = PutProperty(dwDispID, pVar);
		return hr;
	}
	_Check_return_ HRESULT PutProperty(
		_In_ DISPID dwDispID, 
		_In_ VARIANT* pVar) throw()
	{
		return PutProperty(p, dwDispID, pVar);
	}
	_Check_return_ HRESULT GetIDOfName(
		_In_z_ LPCOLESTR lpsz, 
		_Out_ DISPID* pdispid) throw()
	{
		return p->GetIDsOfNames(IID_NULL, const_cast<LPOLESTR*>(&lpsz), 1, LOCALE_USER_DEFAULT, pdispid);
	}
	// Invoke a method by DISPID with no parameters
	_Check_return_ HRESULT Invoke0(
		_In_ DISPID dispid, 
		_Out_opt_ VARIANT* pvarRet = NULL) throw()
	{
		DISPPARAMS dispparams = { NULL, NULL, 0, 0};
		return p->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispparams, pvarRet, NULL, NULL);
	}
	// Invoke a method by name with no parameters
	_Check_return_ HRESULT Invoke0(
		_In_z_ LPCOLESTR lpszName, 
		_Out_opt_ VARIANT* pvarRet = NULL) throw()
	{
		HRESULT hr;
		DISPID dispid;
		hr = GetIDOfName(lpszName, &dispid);
		if (SUCCEEDED(hr))
			hr = Invoke0(dispid, pvarRet);
		return hr;
	}
	// Invoke a method by DISPID with a single parameter
	_Check_return_ HRESULT Invoke1(
		_In_ DISPID dispid, 
		_In_ VARIANT* pvarParam1, 
		_Out_opt_ VARIANT* pvarRet = NULL) throw()
	{
		DISPPARAMS dispparams = { pvarParam1, NULL, 1, 0};
		return p->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispparams, pvarRet, NULL, NULL);
	}
	// Invoke a method by name with a single parameter
	_Check_return_ HRESULT Invoke1(
		_In_z_ LPCOLESTR lpszName, 
		_In_ VARIANT* pvarParam1, 
		_Out_opt_ VARIANT* pvarRet = NULL) throw()
	{		 
		DISPID dispid;
		HRESULT hr = GetIDOfName(lpszName, &dispid);
		if (SUCCEEDED(hr))
			hr = Invoke1(dispid, pvarParam1, pvarRet);
		return hr;
	}
	// Invoke a method by DISPID with two parameters
	_Check_return_ HRESULT Invoke2(
		_In_ DISPID dispid, 
		_In_ VARIANT* pvarParam1, 
		_In_ VARIANT* pvarParam2, 
		_Out_opt_ VARIANT* pvarRet = NULL) throw();
	// Invoke a method by name with two parameters
	_Check_return_ HRESULT Invoke2(
		_In_z_ LPCOLESTR lpszName, 
		_In_ VARIANT* pvarParam1, 
		_In_ VARIANT* pvarParam2, 
		_Out_opt_ VARIANT* pvarRet = NULL) throw()
	{
		DISPID dispid;
		HRESULT hr = GetIDOfName(lpszName, &dispid);
		if (SUCCEEDED(hr))
			hr = Invoke2(dispid, pvarParam1, pvarParam2, pvarRet);
		return hr;
	}
	// Invoke a method by DISPID with N parameters
	_Check_return_ HRESULT InvokeN(
		_In_ DISPID dispid, 
		_In_ VARIANT* pvarParams, 
		_In_ UINT nParams, 
		_Out_opt_ VARIANT* pvarRet = NULL) throw()
	{
		DISPPARAMS dispparams = { pvarParams, NULL, static_cast<UINT>(nParams), 0};
		return p->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispparams, pvarRet, NULL, NULL);
	}
	// Invoke a method by name with Nparameters
	_Check_return_ HRESULT InvokeN(
		_In_z_ LPCOLESTR lpszName, 
		_In_ VARIANT* pvarParams, 
		_In_ int nParams, 
		_Out_opt_ VARIANT* pvarRet = NULL) throw()
	{
		HRESULT hr;
		DISPID dispid;
		hr = GetIDOfName(lpszName, &dispid);
		if (SUCCEEDED(hr))
			hr = InvokeN(dispid, pvarParams, nParams, pvarRet);
		return hr;
	}
	_Check_return_ static HRESULT PutProperty(
		_In_ IDispatch* p, 
		_In_ DISPID dwDispID, 
		_In_ VARIANT* pVar) throw()
	{
		ATLASSERT(p);
		ATLASSERT(pVar != NULL);
		if (pVar == NULL)
			return E_POINTER;
		
		if(p == NULL)
			return E_INVALIDARG;
		
		//ATLTRACE(atlTraceCOM, 2, _T("CPropertyHelper::PutProperty\n"));
		DISPPARAMS dispparams = {NULL, NULL, 1, 1};
		dispparams.rgvarg = pVar;
		DISPID dispidPut = DISPID_PROPERTYPUT;
		dispparams.rgdispidNamedArgs = &dispidPut;

		if (pVar->vt == VT_UNKNOWN || pVar->vt == VT_DISPATCH || 
			(pVar->vt & VT_ARRAY) || (pVar->vt & VT_BYREF))
		{
			HRESULT hr = p->Invoke(dwDispID, IID_NULL,
				LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF,
				&dispparams, NULL, NULL, NULL);
			if (SUCCEEDED(hr))
				return hr;
		}
		return p->Invoke(dwDispID, IID_NULL,
				LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT,
				&dispparams, NULL, NULL, NULL);
	}
	_Check_return_ static HRESULT GetProperty(
		_In_ IDispatch* p, 
		_In_ DISPID dwDispID, 
		_Out_ VARIANT* pVar) throw()
	{
		ATLASSERT(p);
		ATLASSERT(pVar != NULL);
		if (pVar == NULL)
			return E_POINTER;
		
		if(p == NULL)
			return E_INVALIDARG;
			
		//ATLTRACE(atlTraceCOM, 2, _T("CPropertyHelper::GetProperty\n"));
		DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
		return p->Invoke(dwDispID, IID_NULL,
				LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
				&dispparamsNoArgs, pVar, NULL, NULL);
	}
};

template <class T, const IID* piid = &__uuidof(T)>
class CComQIPtr : 
	public CComPtr<T>
{
public:
	CComQIPtr() throw()
	{
	}
	CComQIPtr(_Inout_opt_ T* lp) throw() :
		CComPtr<T>(lp)
	{
	}
	CComQIPtr(_Inout_ const CComQIPtr<T,piid>& lp) throw() :
		CComPtr<T>(lp.p)
	{
	}
	CComQIPtr(_Inout_opt_ IUnknown* lp) throw()
	{
		if (lp != NULL)
			lp->QueryInterface(*piid, (void **)&p);
	}
	T* operator=(_Inout_opt_ T* lp) throw()
	{
        if(*this!=lp)
        {
		    return static_cast<T*>(AtlComPtrAssign((IUnknown**)&p, lp));
        }
        return *this;
	}
	T* operator=(_Inout_ const CComQIPtr<T,piid>& lp) throw()
	{
        if(*this!=lp)
        {
    		return static_cast<T*>(AtlComPtrAssign((IUnknown**)&p, lp.p));
        }
        return *this;
	}
	T* operator=(_Inout_opt_ IUnknown* lp) throw()
	{
        if(*this!=lp)
        {
    		return static_cast<T*>(AtlComQIPtrAssign((IUnknown**)&p, lp, *piid));
        }
        return *this;
	}
};

//Specialization to make it work
template<>
class CComQIPtr<IUnknown, &IID_IUnknown> : 
	public CComPtr<IUnknown>
{
public:
	CComQIPtr() throw()
	{
	}
	CComQIPtr(_Inout_opt_ IUnknown* lp) throw()
	{
		//Actually do a QI to get identity
		if (lp != NULL)
			lp->QueryInterface(__uuidof(IUnknown), (void **)&p);
	}
	CComQIPtr(_Inout_ const CComQIPtr<IUnknown,&IID_IUnknown>& lp) throw() :
		CComPtr<IUnknown>(lp.p)
	{
	}
	IUnknown* operator=(_Inout_opt_ IUnknown* lp) throw()
	{
        if(*this!=lp)
        {
		    //Actually do a QI to get identity
		    return AtlComQIPtrAssign((IUnknown**)&p, lp, __uuidof(IUnknown));
        }
        return *this;
	}

	IUnknown* operator=(_Inout_ const CComQIPtr<IUnknown,&IID_IUnknown>& lp) throw()
	{
        if(*this!=lp)
        {
    		return AtlComPtrAssign((IUnknown**)&p, lp.p);
        }
        return *this;
	}
};
