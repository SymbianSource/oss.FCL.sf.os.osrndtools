/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  
*
*/


#ifndef PIPROFILER_BUPECOM_SAMPLER_H
#define PIPROFILER_BUPECOM_SAMPLER_H

#include <w32std.h>
#include <w32std.h>			// RWsSession
#include <w32adll.h>		// RAnim DLL
#include <e32std.h>
#include <e32property.h>	// RProperty

#include <piprofiler/ProfilerTraces.h>
#include <piprofiler/ProfilerConfig.h>
#include <piprofiler/ProfilerVersion.h>
#include <piprofiler/SamplerPluginInterface.h>
#include <piprofiler/ProfilerGenericClassesUsr.h>

#include <data_caging_path_literals.hrh> // for KDC_SHARED_LIB_DIR

// Button press&touch event Anim DLL interface
#include "TouchEventClientDll.h"

// caption definitions
_LIT8(KBUPShortName, "bup");
_LIT8(KBUPLongName, "Button and touch event capture");
_LIT8(KBUPDescription, "Button and touch event sampler\nTracing button and touch screen events\nHW dep: N/A\nSW dep: S60 3.0\n");

const TUid KProfilerKeyEventPropertyCat={0x2001E5AD};
enum TProfilerKeyEventPropertyKeys
	{
	EProfilerKeyEventPropertySample = 7
	};

const TUid KGppPropertyCat={0x20201F70};
enum TGppPropertyKeys
	{
	EGppPropertySyncSampleNumber
	};


static _LIT_SECURITY_POLICY_PASS(KAllowAllPolicy);
static _LIT_SECURITY_POLICY_C1(KCapabilityNone, ECapability_None);

_LIT(KDllName, "PIProfilerTouchEventAnim.DLL");	// animation server dll	on user disk

/*
 *	
 *	BUP sampler definition
 *	
 */
class CProfilerButtonListener;
class CSamplerPluginInterface;

class CBupPlugin : public CSamplerPluginInterface
    {
public:	
	static CBupPlugin* NewL(const TUid aImplementationUid, TAny* aInitParams);
			~CBupPlugin();

	TInt	ResetAndActivateL(CProfilerSampleStream& aStream);
	TInt	StopSampling();
    TBool   Enabled() { return iEnabled; }

	TInt	CreateFirstSample();

	// no sub samplers, from CSamplerPluginInterface
	TInt 	SubId(TUid /*aId*/) const {return KErrNotFound;}
    TInt    GetSamplerType();
	
	void    GetAttributesL(CArrayFixFlat<TSamplerAttributes>* aAttributes);
    TInt    SetAttributesL(TSamplerAttributes& aAttributes);
    void    InitiateSamplerAttributesL();
	
    TInt    ConvertRawSettingsToAttributes(CDesC8ArrayFlat* aSingleSettingArray);
    
    TInt    DoSetSamplerSettings(CDesC8ArrayFlat* aAllSettings, TDesC8& aSamplerName, TInt aIndex);
    void    SaveSettingToAttributes(const TDesC8& aSetting, TInt aIndex);
    
	TUid  Id(TInt aSubId) const;

	void FillThisStreamBuffer(TBapBuf* nextFree,TRequestStatus& aStatus); 
	
private:
			CBupPlugin();
	void 	ConstructL();

private:
	TUint8					iVersion[20];
	TPtr8					iVersionDescriptor;
	
	TInt 					iSamplerType;

	CProfilerButtonListener* 	iButtonListener;
    CArrayFixFlat<TSamplerAttributes>* iSamplerAttributes;
public:
	TUint32* 				iSampleTime;
    };

/*
*
*  Base class for all windows
*
*/
class CWsClient : public CActive
	{
	protected:
		//construct
		CWsClient();
		CWsScreenDevice* iScreen;
		RWsSession iWs;
	public:
		void ConstructL(CBupPlugin* aSampler);
		// destruct
		~CWsClient();
		// main window
		virtual void ConstructMainWindowL();
		void Exit();
		// active object protocol
		void IssueRequest(); // request an event
		void DoCancel(); // cancel the request
		virtual void RunL() = 0; // handle completed request
		virtual void HandleKeyEventL (TKeyEvent& aKeyEvent) = 0;
		virtual void HandleEvent(TInt c) = 0;
		RWindowGroup Group() {return iGroup;};
		CBupPlugin* GetSampler();
    private:
		RWindowGroup                    iGroup;
		CWindowGc*                      iGc;
		friend class                    CWindow; // needs to get at session
		RProperty                       iProperty;
	    CBupPlugin*                     iSampler;
	};

/*
*
*  CWindow declaration
*
*/
class CWindow : public CBase
	{
protected:
    RWindow iWindow; 	// window server window
    TRect iRect; 		// rectangle re owning window
public:
    CWindow(CWsClient* aClient);
    void ConstructL (const TRect& aRect, CWindow* aParent=0);
    ~CWindow();
    // access
    RWindow& Window(); // our own window
    CWindowGc* SystemGc(); // system graphics context
    // empty drawing functions
    virtual void Draw(const TRect& aRect) = 0;
    virtual void HandlePointerEvent (TPointerEvent& aPointerEvent) = 0;
    
    CWsClient* Client() {return iClient;};
private:
    CWsClient*      iClient; // client including session and group
	};

class CMainWindow : public CWindow
    {
public:
    CMainWindow (CWsClient* aClient);
    void Draw (const TRect& aRect);
    ~CMainWindow ();
    void HandlePointerEvent (TPointerEvent& aPointerEvent);
    void ConstructL (const TRect& aRect, CWindow* aParent=0);
    };

class CProfilerButtonListener : public CWsClient 
    {
public:
    CProfilerButtonListener();
    void    ConstructMainWindowL();
    ~CProfilerButtonListener();
    void    RunL();
    void    DoCancel();
    void    HandleKeyEventL (TKeyEvent& aKeyEvent);
    void    HandleEvent(TInt c);
private:
    CMainWindow*     iMainWindow;    // main window
    TInt             iSampleStartTime;
    TUint8                          iSample[8];
    RProfilerTouchEventAnim*        iAnim;
    RAnimDll*                       iAnimDll;    
    };

#endif
