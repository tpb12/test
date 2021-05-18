#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSettings : public CObject
{
	DECLARE_SERIAL(CSettings);

public:
	CSettings();
	virtual ~CSettings();

public:
	BOOL Load(LPCTSTR lpszProfileName);
	BOOL Save(LPCTSTR lpszProfileName);
	void MakeStatusMsg();
	void UpdateStatusMsg(unsigned char);
public:
	DWORD m_dwPollingPeriod;
	BOOL m_bTestLoopback;
	BOOL m_bShowSIOMessages;
	BOOL m_bShowMessageErrors;
	BOOL m_bShowCOMErrors;
	CString m_strSettingsReportPath;

	UINT m_nBufferSize;

	CString m_strIncomingAddress;
	UINT m_nIncomingPort;

	CString m_strCOMSetup;
	int m_iCOMRttc;
	int m_iCOMWttc;
	int m_iCOMRit;

	CByteArray m_arPrefix;
	CByteArray m_arOutPrefix;

	WORD m_wComposedType;
	WORD m_wOutputComposedType;
	WORD m_wCRC16Init;
	WORD m_wCPAddr;
	WORD m_wPUAddr;
	typedef struct tagMESSAGETYPE
	{
		WORD m_wType;			//���
		WORD m_wMaxLength;		//max ����� ��������� ��� ����
		WORD m_wDestination;	//����������
		WORD m_wSource;			//�����������
		WORD m_wDestMask;		//����� ���������� (������� ����������)
		WORD m_wSrcMask;		//����� ����������� (������� ����������)
		tagMESSAGETYPE(WORD wType = 0, WORD wMaxLength = 0, WORD wDestination = 0, WORD wSource = 0, WORD wDestMask = 0, WORD wSrcMask = 0)
		{
			m_wType = wType;
			m_wMaxLength = wMaxLength;
			m_wDestination = wDestination;
			m_wSource = wSource;
			m_wDestMask = wDestMask;
			m_wSrcMask = wSrcMask;
		}
	} MESSAGETYPE, * PMESSAGETYPE;
	CMap<WORD, WORD, MESSAGETYPE, MESSAGETYPE&> m_mapMsgTypes;
	CMapWordToPtr m_mapMsgTypesToUnpack;
	BOOL m_bUnpackAll;
	CMapWordToPtr m_mapMsgTypesToMark;
	BOOL m_bMarkAll;

	UINT m_nStatusPeriod;
	int m_iSendStatTO;
	MESSAGETYPE m_StatusHdr;
	MESSAGETYPE m_StatusMsg;
	MESSAGETYPE m_MarkNestedMask;
	MESSAGETYPE m_MarkComposedMask;
	WORD m_TUType;
	WORD m_TUSrcMask;
	BOOL m_TUSrcComMsgIndex;
	UINT m_TUPrimToSecSrc;
	UINT m_TUSecToPrimSrc;

	CByteArray m_arStatusData;
	CByteArray m_arStatusMsg;

	BOOL m_bKeepLog;
	WORD m_wLogComposedType;
	CMapWordToPtr m_mapLogMsgTypesToUnpack;
	BOOL m_bLogUnpackAll;

	WORD m_wLogComposedTypeToPack;
	CMapWordToPtr m_mapLogMsgTypesToPack;
	BOOL m_bLogPackAll;
	
	//������ ugs
	WORD m_wSourceID;
	WORD m_wStatusRequestMessageType;
};

extern CSettings g_Settings;

#endif // _SETTINGS_H_
