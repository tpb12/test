#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSettings
{
public:
	typedef std::map<WORD, void*> MapWordToPtr;
	typedef std::vector<BYTE> ByteArray;

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
	std::string m_strSettingsReportPath;

	UINT m_nBufferSize;

	std::string m_strIncomingAddress;
	UINT m_nIncomingPort;

	std::string m_strCOMSetup;
	int m_iCOMRttc;
	int m_iCOMWttc;
	int m_iCOMRit;

	ByteArray m_arPrefix;
	ByteArray m_arOutPrefix;

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
	//CMap<WORD, WORD, MESSAGETYPE, MESSAGETYPE&> m_mapMsgTypes;
	std::map<WORD, MESSAGETYPE>m_mapMsgTypes;
	MapWordToPtr m_mapMsgTypesToUnpack;
	BOOL m_bUnpackAll;
	MapWordToPtr m_mapMsgTypesToMark;
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

	ByteArray m_arStatusData;
	ByteArray m_arStatusMsg;

	BOOL m_bKeepLog;
	WORD m_wLogComposedType;
	MapWordToPtr m_mapLogMsgTypesToUnpack;
	BOOL m_bLogUnpackAll;

	WORD m_wLogComposedTypeToPack;
	MapWordToPtr m_mapLogMsgTypesToPack;
	BOOL m_bLogPackAll;
	
	//������ ugs
	WORD m_wSourceID;
	WORD m_wStatusRequestMessageType;
private:
	static bool SettingsLookup(const std::map<std::string, std::string>& mapset,
		const std::string& sstring, std::string& str)
	{
		const auto& it = mapset.find(sstring);
		if (it != mapset.end())
		{
			str = it->second;
			return true;
		}
		return false;
	}
	static void trim(std::string& s)
	{
		s.erase(s.begin(), std::find_if_not(s.begin(), s.end(),
			[](char c) { return std::isspace(c); }));
		s.erase(std::find_if_not(s.rbegin(), s.rend(),
			[](char c) { return std::isspace(c); }).base(), s.end());
	}
	static void trim(std::string& s, char space)
	{
		s.erase(s.begin(), std::find_if_not(s.begin(), s.end(),
			[space](char c) { return c == space; }));
		s.erase(std::find_if_not(s.rbegin(), s.rend(),
			[space](char c) { return c == space; }).base(), s.end());
	}
	static std::vector<std::string> tokenize(const std::string& input,
		const char token)
	{
		std::stringstream ss(input);
		std::string s;
		std::vector<std::string> result;

		while (std::getline(ss, s, token))
		{
			result.push_back(s);
		}

		return result;
	}
};

extern CSettings g_Settings;

#endif // _SETTINGS_H_
