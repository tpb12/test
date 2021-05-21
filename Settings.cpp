#include "stdafx.h"
#include "Settings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSettings g_Settings;

//IMPLEMENT_SERIAL(CSettings, CObject, 1)

CSettings::CSettings()
{
	m_dwPollingPeriod = 1000;
	m_bTestLoopback = FALSE;
	m_bShowSIOMessages = FALSE;
	m_bShowMessageErrors = FALSE;
	m_bShowCOMErrors = FALSE;
	m_strSettingsReportPath = _T("ugs.rep");

	m_nBufferSize = 0x90000;

	m_nIncomingPort = 11112;

	m_strCOMSetup = _T("COM1: baud=9600 data=8 parity=N stop=1");
	m_iCOMRttc = 0;
	m_iCOMWttc = 0;
	m_iCOMRit = -1;

	m_arPrefix.RemoveAll();

	m_wComposedType = 0x000003;
	m_wOutputComposedType = 0x0000;
	m_wCRC16Init = 0xFFFF;

	m_wCPAddr = 0x0000;
	m_wPUAddr = 0x0000;

	m_bUnpackAll = FALSE;
	m_bMarkAll = FALSE;

	m_nStatusPeriod = 0;
	m_iSendStatTO = 1000000;
	m_StatusHdr = MESSAGETYPE(0x0000, 0x20);
	m_StatusMsg = MESSAGETYPE(m_wComposedType);
	m_MarkNestedMask = MESSAGETYPE();
	m_MarkComposedMask = MESSAGETYPE();
	m_arStatusData.RemoveAll();
	MakeStatusMsg();
	UpdateStatusMsg(0);
	m_TUType = 0x000002;
	m_TUSrcMask = 0x0000;
	m_TUSrcComMsgIndex = FALSE;
	m_TUPrimToSecSrc = 1;
	m_TUSecToPrimSrc = 1;


	m_bKeepLog = FALSE;
	m_wLogComposedType = 0x0000;
	m_bLogUnpackAll = FALSE;

	m_wLogComposedTypeToPack = 0x0000;
	m_bLogUnpackAll = FALSE;

	m_wSourceID = 0x000020;
	m_wStatusRequestMessageType = 0x0001;

	MESSAGETYPE typeStatus(0x0001, 0x1000);
	m_mapMsgTypes.SetAt(0x0001, typeStatus);
}

CSettings::~CSettings()
{
}

void replDef(unsigned short& expl, const unsigned short& def)
{
  expl = expl ? expl : def;
}


BOOL CSettings::Load(LPCTSTR lpszProfileName)
{
	try
	{
		//CStdioFile file(lpszProfileName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText);
		std::ifstream infile(lpszProfileName, std::ios_base::in);
		if (!infile)
			return FALSE;

		CMapStringToString mapSettings;
		std::map<std::string, std::string> mapSettings1;
		CString strGroup(_T("[General]"));
		std::string strGroup1("[General]");

		std::string strLine1;
		while (!infile.eof())
		{
			std::getline(infile, strLine1);
			trim(strLine1);
			std::size_t iComment = strLine1.find(_T(';'), 0);
			if (iComment != std::string::npos)
				strLine1.erase(iComment, strLine1.length() - iComment);
			if (strLine1.empty())
				continue;
			if (strLine1.find(_T('[')) == 0 &&
				strLine1.rfind(_T(']')) == strLine1.length() - 1)
			{
				strGroup1 = strLine1;
				continue;
			}

			int iPos = 0;
			std::string strKey(strGroup1 + _T('/'));
			std::string strValue;
			auto toks = tokenize(strLine1, '=');
			if (toks.size() > 1)
			{
				strKey += toks[0];
				strValue = toks[1];
				trim(strKey);
				trim(strValue);
				trim(strValue, '\"');
			}
			mapSettings1.insert({ strKey, strValue });
		}

		//CString strLine;
		//while(file.ReadString(strLine))
		//{
		//	strLine.Trim();
		//	int iComment = strLine.Find(_T(';'), 0);
		//	if(iComment >= 0)
		//		strLine.Delete(iComment, strLine.GetLength() - iComment);

		//	if(strLine.IsEmpty())
		//		continue;

		//	if(strLine.Find(_T('[')) == 0 && strLine.ReverseFind(_T(']')) == strLine.GetLength() - 1)
		//	{
		//		strGroup = strLine;
		//		continue;
		//	}

		//	int iPos = 0;
		//	CString strKey(strGroup + _T('/') + strLine.Tokenize(_T("="), iPos).Trim());
		//	CString strValue(strLine);
		//	strValue.Delete(0, iPos);

		//	mapSettings.SetAt(strKey, strValue.Trim().Trim(_T("\"")));
		//	mapSettings1.insert({ strKey, strValue });
		//}

		int iValue;
		std::string strLine;
		if (SettingsLookup(mapSettings1, _T("[General]/PollingPeriod"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_DEFAULT, &iValue) && iValue != 0)
			m_dwPollingPeriod = (DWORD)iValue;

		if(SettingsLookup(mapSettings1, _T("[General]/TestLoopback"), strLine))
			m_bTestLoopback = strLine == _T("1") ? TRUE : FALSE;

		if(SettingsLookup(mapSettings1, _T("[General]/ShowSIOMessages"), strLine))
			m_bShowSIOMessages = strLine == _T("1") ? TRUE : FALSE;

		if(SettingsLookup(mapSettings1, _T("[General]/ShowMessageErrors"), strLine))
			m_bShowMessageErrors = strLine == _T("1") ? TRUE : FALSE;

		if(SettingsLookup(mapSettings1, _T("[General]/ShowCOMErrors"), strLine))
			m_bShowCOMErrors = strLine == _T("1") ? TRUE : FALSE;

		if(SettingsLookup(mapSettings1, _T("[General]/SettingsReportPath"), strLine))
			m_strSettingsReportPath = strLine;

		if(SettingsLookup(mapSettings1, _T("[General]/BufferSize"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_SUPPORT_HEX, &iValue) && iValue != 0)
			m_nBufferSize = (UINT)iValue;

		if(SettingsLookup(mapSettings1, _T("[UDP]/IncomingPort"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_DEFAULT, &iValue) && iValue != 0)
			m_nIncomingPort = (UINT)iValue;

		if(SettingsLookup(mapSettings1, _T("[UDP]/OutgoingIP"), strLine))
		{
			DWORD dwTemp = (DWORD)inet_addr(strLine.c_str());
			if(INADDR_NONE == dwTemp)
			{
				LPHOSTENT lphost;
				lphost = gethostbyname(strLine.c_str());
				if (lphost != NULL)
					dwTemp = ((LPIN_ADDR)lphost->h_addr)->s_addr;
				else
				{
					::WSASetLastError(WSAEINVAL);
					dwTemp = (DWORD)ntohl((u_long)INADDR_LOOPBACK);
				}
			}

		}

		if(SettingsLookup(mapSettings1, _T("[COM]/SetupParams"), strLine))
			m_strCOMSetup = strLine;

		if(SettingsLookup(mapSettings1, _T("[COM]/rttc"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_DEFAULT, &iValue))
			m_iCOMRttc = iValue;

		if(SettingsLookup(mapSettings1, _T("[COM]/wttc"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_DEFAULT, &iValue))
			m_iCOMWttc = iValue;

		if(SettingsLookup(mapSettings1, _T("[COM]/rit"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_DEFAULT, &iValue))
			m_iCOMRit = iValue;

		CByteArray arTemp;
		if(SettingsLookup(mapSettings1, _T("[Message]/CPAddr"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_SUPPORT_HEX, &iValue))
			m_wCPAddr = (WORD)iValue;
		if(SettingsLookup(mapSettings1, _T("[Message]/PUAddr"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_SUPPORT_HEX, &iValue))
			m_wPUAddr = (WORD)iValue;

		if(SettingsLookup(mapSettings1, _T("[Message]/Prefix"), strLine) &&
			ByteArrayFromString(strLine.c_str(), arTemp, _T("")))
			m_arPrefix.Copy(arTemp);

		if(SettingsLookup(mapSettings1, _T("[Message]/OutPrefix"), strLine) &&
			ByteArrayFromString(strLine.c_str(), arTemp, _T("")))
			m_arOutPrefix.Copy(arTemp);

		if(SettingsLookup(mapSettings1, _T("[Message]/CRC16Init"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_SUPPORT_HEX, &iValue))
			m_wCRC16Init = (WORD)iValue;

		if(SettingsLookup(mapSettings1, _T("[Message]/ComposedType"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_SUPPORT_HEX, &iValue))
			m_wComposedType = (WORD)iValue;

		if(SettingsLookup(mapSettings1, _T("[Message]/OutputComposedType"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_SUPPORT_HEX, &iValue))
			m_wOutputComposedType = (WORD)iValue;

		if(SettingsLookup(mapSettings1, _T("[Message]/TypesToUnPack"), strLine))
		{
			m_mapMsgTypesToUnpack.RemoveAll();
			if(strLine == _T("*"))
			{
				m_mapMsgTypesToUnpack.SetAt(0x0000, NULL);
				m_bUnpackAll = TRUE;
			}
			else
			{
				auto toks = tokenize(strLine, ' ');
				for (auto& s : toks)
				{
					trim(s);
					if (::StrToIntEx(s.c_str(), STIF_SUPPORT_HEX, &iValue))
						m_mapMsgTypesToUnpack.SetAt((WORD)iValue, NULL);
				}
				/*for(int iPos = 0; iPos < strLine.GetLength() - 1; )
				{
					if(::StrToIntEx(strLine.Tokenize(_T(" "), iPos), STIF_SUPPORT_HEX, &iValue))
						m_mapMsgTypesToUnpack.SetAt((WORD)iValue, NULL);
				}*/
				m_bUnpackAll = FALSE;
			}
		}

		if(SettingsLookup(mapSettings1, _T("[Message]/MarkComposedMessageMask"), strLine))
		{
			auto toks = tokenize(strLine, ' ');
			if (toks.size() > 1)
			{
				if (::StrToIntEx(toks[0].c_str(), STIF_SUPPORT_HEX, &iValue))
					m_MarkComposedMask.m_wDestMask = (WORD)iValue;
				if (::StrToIntEx(toks[1].c_str(), STIF_SUPPORT_HEX, &iValue))
					m_MarkComposedMask.m_wSrcMask = (WORD)iValue;
			}
			/*int iPos = 0;
			if(::StrToIntEx(strLine.Tokenize(_T(" "), iPos), STIF_SUPPORT_HEX, &iValue))
				m_MarkComposedMask.m_wDestMask = (WORD)iValue;
			if(::StrToIntEx(strLine.Tokenize(_T(" "), iPos), STIF_SUPPORT_HEX, &iValue))
				m_MarkComposedMask.m_wSrcMask = (WORD)iValue;*/
		}
		if(SettingsLookup(mapSettings1, _T("[Message]/MarkMessageMask"), strLine))
		{
			auto toks = tokenize(strLine, ' ');
			if (toks.size() > 1)
			{
				if (::StrToIntEx(toks[0].c_str(), STIF_SUPPORT_HEX, &iValue))
					m_MarkNestedMask.m_wDestMask = (WORD)iValue;
				if (::StrToIntEx(toks[1].c_str(), STIF_SUPPORT_HEX, &iValue))
					m_MarkNestedMask.m_wSrcMask = (WORD)iValue;
			}
			/*int iPos = 0;
			if(::StrToIntEx(strLine.Tokenize(_T(" "), iPos), STIF_SUPPORT_HEX, &iValue))
				m_MarkNestedMask.m_wDestMask = (WORD)iValue;
			if(::StrToIntEx(strLine.Tokenize(_T(" "), iPos), STIF_SUPPORT_HEX, &iValue))
				m_MarkNestedMask.m_wSrcMask = (WORD)iValue;*/
		}

		if(SettingsLookup(mapSettings1, _T("[Message]/TypesToMark"), strLine))
		{
			m_mapMsgTypesToMark.RemoveAll();
			if(strLine == _T("*"))
			{
				m_mapMsgTypesToMark.SetAt(0x0000, NULL);
				m_bMarkAll = TRUE;
			}
			else
			{
				auto toks = tokenize(strLine, ' ');
				for (auto& s : toks)
				{
					trim(s);
					if (::StrToIntEx(s.c_str(), STIF_SUPPORT_HEX, &iValue))
						m_mapMsgTypesToMark.SetAt((WORD)iValue, NULL);
				}
				//for(int iPos = 0; iPos < strLine.GetLength() - 1; )
				//{
				//	if(::StrToIntEx(strLine.Tokenize(_T(" "), iPos), STIF_SUPPORT_HEX, &iValue))
				//		m_mapMsgTypesToMark.SetAt((WORD)iValue, NULL);
				//}
				m_bMarkAll = FALSE;
			}
		}

		m_mapMsgTypes.RemoveAll();

		for(int i = 1; i < 10; i++)
		{
			//CString strTemp;
			//strTemp.Format(_T("[Message]/Type%u"), i);
			char strTemp[100];
			snprintf(strTemp, sizeof(strTemp), "[Message]/Type%u", i);

			if(!SettingsLookup(mapSettings1, strTemp, strLine))
				continue;

			auto toks = tokenize(strLine, ' ');
			if (toks.size() < 6)
				continue;

			MESSAGETYPE type;
			
			if(::StrToIntEx(toks[0].c_str(), STIF_SUPPORT_HEX, &iValue))
				type.m_wType = (WORD)iValue;
			if(::StrToIntEx(toks[1].c_str(), STIF_SUPPORT_HEX, &iValue))
				type.m_wMaxLength = (WORD)iValue;
			if(::StrToIntEx(toks[2].c_str(), STIF_SUPPORT_HEX, &iValue))
				type.m_wDestination = (WORD)iValue;
			if(::StrToIntEx(toks[3].c_str(), STIF_SUPPORT_HEX, &iValue))
				type.m_wSource = (WORD)iValue;
			if(::StrToIntEx(toks[4].c_str(), STIF_SUPPORT_HEX, &iValue))
				type.m_wDestMask = (WORD)iValue;
			if(::StrToIntEx(toks[5].c_str(), STIF_SUPPORT_HEX, &iValue))
				type.m_wSrcMask = (WORD)iValue;

			if(type.m_wType == 0x0505) {
				replDef(type.m_wSource, m_wPUAddr);
				replDef(type.m_wDestination, m_wCPAddr);
			}
			else if(type.m_wType == 0x0521 || type.m_wType == 0x0532) {
				replDef(type.m_wSource, m_wCPAddr);
			}
			else {
				replDef(type.m_wSource, m_wCPAddr);
				replDef(type.m_wDestination, m_wPUAddr);
			}
			
			m_mapMsgTypes.SetAt(type.m_wType, type);            
		}

		MESSAGETYPE typeStatus(0x0001, 0x1000);
		m_mapMsgTypes.SetAt(0x0001, typeStatus);

		if(SettingsLookup(mapSettings1, _T("[Message]/StatusPeriod"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_DEFAULT, &iValue))
			m_nStatusPeriod = (UINT)iValue;

		if(SettingsLookup(mapSettings1, _T("[Message]/SendStatusTO"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_DEFAULT, &iValue))
			m_iSendStatTO = (int)iValue;

		if(SettingsLookup(mapSettings1, _T("[Message]/StatusMsg"), strLine))
		{
			auto toks = tokenize(strLine, ' ');
			if (toks.size() >= 6)
			{
				if (::StrToIntEx(toks[0].c_str(), STIF_SUPPORT_HEX, &iValue))
					m_StatusHdr.m_wType = (WORD)iValue;
				if (::StrToIntEx(toks[1].c_str(), STIF_SUPPORT_HEX, &iValue))
					m_StatusHdr.m_wDestination = (WORD)iValue;
				if (::StrToIntEx(toks[2].c_str(), STIF_SUPPORT_HEX, &iValue))
					m_StatusHdr.m_wSource = (WORD)iValue;
				if (::StrToIntEx(toks[3].c_str(), STIF_SUPPORT_HEX, &iValue))
					m_StatusMsg.m_wType = (WORD)iValue;
				if (::StrToIntEx(toks[4].c_str(), STIF_SUPPORT_HEX, &iValue))
					m_StatusMsg.m_wDestination = (WORD)iValue;
				if (::StrToIntEx(toks[5].c_str(), STIF_SUPPORT_HEX, &iValue))
					m_StatusMsg.m_wSource = (WORD)iValue;

				if (toks.size() >= 7)
				{
					ByteArrayFromString(toks[6].c_str(), arTemp, _T(""));
					m_arStatusData.Copy(arTemp);
				}
			}
			replDef(m_StatusMsg.m_wSource, m_wCPAddr);
			replDef(m_StatusMsg.m_wDestination, m_wPUAddr);

			MakeStatusMsg();
			UpdateStatusMsg(0);
		}

		if(SettingsLookup(mapSettings1, _T("[Message]/TUType"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_SUPPORT_HEX, &iValue))
			m_TUType = (WORD)iValue;

		if(SettingsLookup(mapSettings1, _T("[Message]/TUSrcMask"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_SUPPORT_HEX, &iValue))
			m_TUSrcMask = (WORD)iValue;
		if(SettingsLookup(mapSettings1, _T("[Message]/TUSrcComMsgIndex"), strLine))
			m_TUSrcComMsgIndex = strLine == _T("1") ? TRUE : FALSE;
		if(SettingsLookup(mapSettings1, _T("[Message]/TUPrimToSecSrc"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_DEFAULT, &iValue))
			m_TUPrimToSecSrc = (UINT)iValue;
		if(SettingsLookup(mapSettings1, _T("[Message]/TUSecToPrimSrc"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_DEFAULT, &iValue))
			m_TUSecToPrimSrc = (UINT)iValue;

		if(SettingsLookup(mapSettings1, _T("[Log]/KeepLog"), strLine))
			m_bKeepLog = strLine == _T("1") ? TRUE : FALSE;

		if(SettingsLookup(mapSettings1, _T("[Log]/LogIP"), strLine))
		{
			DWORD dwTemp = (DWORD)inet_addr(strLine.c_str());
			if(INADDR_NONE == dwTemp)
			{
				LPHOSTENT lphost;
				lphost = gethostbyname(strLine.c_str());
				if (lphost != NULL)
					dwTemp = ((LPIN_ADDR)lphost->h_addr)->s_addr;
				else
				{
					::WSASetLastError(WSAEINVAL);
					dwTemp = (DWORD)ntohl((u_long)INADDR_LOOPBACK);
				}
			}
		}


		if(SettingsLookup(mapSettings1, _T("[Log]/LogComposedType"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_SUPPORT_HEX, &iValue))
			m_wLogComposedType = (WORD)iValue;

		if(SettingsLookup(mapSettings1, _T("[Log]/LogTypesToUnPack"), strLine))
		{
			m_mapLogMsgTypesToUnpack.RemoveAll();
			if(strLine == _T("*"))
			{
				m_mapLogMsgTypesToUnpack.SetAt(0x0000, NULL);
				m_bLogUnpackAll = TRUE;
			}
			else
			{
				auto toks = tokenize(strLine, ' ');
				for (auto& s : toks)
				{
					trim(s);
					if (::StrToIntEx(s.c_str(), STIF_SUPPORT_HEX, &iValue))
						m_mapLogMsgTypesToUnpack.SetAt((WORD)iValue, NULL);
				}
				/*for(int iPos = 0; iPos < strLine.GetLength() - 1; )
				{
					if(::StrToIntEx(strLine.Tokenize(_T(" "), iPos), STIF_SUPPORT_HEX, &iValue))
						m_mapLogMsgTypesToUnpack.SetAt((WORD)iValue, NULL);
				}*/
				m_bLogUnpackAll = FALSE;
			}
		}

		if(SettingsLookup(mapSettings1, _T("[Log]/LogComposedTypeToPack"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_SUPPORT_HEX, &iValue))
			m_wLogComposedTypeToPack = (WORD)iValue;

		if(SettingsLookup(mapSettings1, _T("[Log]/LogTypesToPack"), strLine))
		{
			m_mapLogMsgTypesToPack.RemoveAll();
			if(strLine == _T("*"))
			{
				m_mapLogMsgTypesToPack.SetAt(0x0000, NULL);
				m_bLogPackAll = TRUE;
			}
			else
			{
				auto toks = tokenize(strLine, ' ');
				for (auto& s : toks)
				{
					trim(s);
					if (::StrToIntEx(s.c_str(), STIF_SUPPORT_HEX, &iValue))
						m_mapLogMsgTypesToPack.SetAt((WORD)iValue, NULL);
				}
				/*for(int iPos = 0; iPos < strLine.GetLength() - 1; )
				{
					if(::StrToIntEx(strLine.Tokenize(_T(" "), iPos), STIF_SUPPORT_HEX, &iValue))
						m_mapLogMsgTypesToPack.SetAt((WORD)iValue, NULL);
				}*/
				m_bLogPackAll = FALSE;
			}
		}

		if(SettingsLookup(mapSettings1, _T("[Status]/SourceIndex"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_SUPPORT_HEX, &iValue))
			m_wSourceID = (WORD)iValue;

		if(SettingsLookup(mapSettings1, _T("[Status]/StatusRequestMessageType"), strLine) &&
			::StrToIntEx(strLine.c_str(), STIF_SUPPORT_HEX, &iValue))
			m_wStatusRequestMessageType = (WORD)iValue;

		return TRUE;
	}
	catch (std::exception&)
	{
	}

	return FALSE;
}

BOOL CSettings::Save(LPCTSTR lpszProfileName)
{
	try
	{
		std::ofstream outfile(lpszProfileName);//, std::ios_base::out);
		if (!outfile.is_open())
			return FALSE;

		//CStdioFile file(lpszProfileName, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeText);
		//CString strTemp;

		TCHAR strTemp[2048];
		snprintf(strTemp, sizeof(strTemp), _T("[General]\n"));
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("PollingPeriod = %u\n"), m_dwPollingPeriod);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("TestLoopback = %s\n"), m_bTestLoopback ? _T("1") : _T("0"));
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("ShowSIOMessages = %s\n"), m_bShowSIOMessages ? _T("1") : _T("0"));
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("ShowMessageErrors = %s\n"), m_bShowMessageErrors ? _T("1") : _T("0"));
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("ShowCOMErrors = %s\n"), m_bShowCOMErrors ? _T("1") : _T("0"));
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("SettingsReportPath = \"%s\"\n"), m_strSettingsReportPath.c_str());
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("BufferSize = 0x%04X\n"), m_nBufferSize);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("[UDP]\n"));
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("IncomingPort = %u\n"), m_nIncomingPort);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("[COM]\n"));
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("SetupParams = \"%s\"\n"), m_strCOMSetup.c_str());
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("rttc = %i\n"), m_iCOMRttc);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("wttc = %i\n"), m_iCOMWttc);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("rit = %i\n"), m_iCOMRit);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("[Message]\n"));
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("CPAddr = 0x%04X\n"), m_wCPAddr);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("PUAddr = 0x%04X\n"), m_wPUAddr);
		outfile << strTemp;
		std::string stemp;
		ByteArrayToString(m_arPrefix.GetData(), (int)m_arPrefix.GetSize(), stemp, _T("Prefix = \""));
		outfile << stemp <<_T("\"\n");
		snprintf(strTemp, sizeof(strTemp), _T("CRC16Init = 0x%04X\n"), m_wCRC16Init);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("ComposedType = 0x%04X\n"), m_wComposedType);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("OutputComposedType = 0x%04X\n"), m_wOutputComposedType);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("TypesToUnPack = \""));
		outfile << strTemp;
		if(m_bUnpackAll)
			outfile << _T("*");
		else
		{
			POSITION pos = m_mapMsgTypesToUnpack.GetStartPosition();
			while(pos != NULL)
			{
				WORD wType;
				void * pTemp;
				m_mapMsgTypesToUnpack.GetNextAssoc(pos, wType, pTemp);
				snprintf(strTemp, sizeof(strTemp), _T("%04X "), wType);
				outfile << strTemp;
			}
		}
		outfile << _T("\"\n");

		snprintf(strTemp, sizeof(strTemp), _T("MarkComposedMessageMask  = \"0x%04X 0x%04X\"\n"), m_MarkComposedMask.m_wDestMask,
			m_MarkComposedMask.m_wSrcMask);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("MarkMessageMask  = \"0x%04X 0x%04X\"\n"), m_MarkNestedMask.m_wDestMask,
			m_MarkNestedMask.m_wSrcMask);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("TypesToMark = \""));
		outfile << strTemp;
		if(m_bMarkAll)
			outfile << _T("*");
		else
		{
			POSITION pos = m_mapMsgTypesToMark.GetStartPosition();
			while(pos != NULL)
			{
				WORD wType;
				void * pTemp;
				m_mapMsgTypesToMark.GetNextAssoc(pos, wType, pTemp);
				snprintf(strTemp, sizeof(strTemp), _T("0x%04X "), wType);
				outfile << strTemp;
			}
		}
		outfile << _T("\"\n");

		POSITION pos = m_mapMsgTypes.GetStartPosition();
		for(int i = 0; pos != NULL && i < 10; i++)
		{
			WORD wType;
			MESSAGETYPE type;
			m_mapMsgTypes.GetNextAssoc(pos, wType, type);
			snprintf(strTemp, sizeof(strTemp), _T("Type%u = \"0x%04X 0x%X 0x%04X 0x%04X 0x%04X 0x%04X\"\n"), i, type.m_wType,
				type.m_wMaxLength, type.m_wDestination, type.m_wSource, type.m_wDestMask, type.m_wSrcMask);
			outfile << strTemp;
		}

		snprintf(strTemp, sizeof(strTemp), _T("StatusPeriod = %u\n"), m_nStatusPeriod);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("SendStatusTO = %u\n"), m_iSendStatTO);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("StatusMsg = \"0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X "), m_StatusHdr.m_wType,
			m_StatusHdr.m_wDestination, m_StatusHdr.m_wSource, m_StatusMsg.m_wType, m_StatusMsg.m_wDestination,
			m_StatusMsg.m_wSource);
		outfile << strTemp;
		ByteArrayToString(m_arStatusData.GetData(), m_arStatusData.GetSize(), stemp);
		outfile << stemp << _T("\"\n");

		snprintf(strTemp, sizeof(strTemp), _T("TUType = 0x%04X\n"), m_TUType);
		outfile << strTemp;

		snprintf(strTemp, sizeof(strTemp), _T("TUSrcMask = 0x%04X\n"), m_TUSrcMask);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("TUSrcComMsgIndex = %s\n"), m_TUSrcComMsgIndex ? _T("1") : _T("0"));
		outfile << strTemp;

		snprintf(strTemp, sizeof(strTemp), _T("TUPrimToSecSrc = %u\n"), m_TUPrimToSecSrc);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("TUSecToPrimSrc = %u\n"), m_TUSecToPrimSrc);
		outfile << strTemp;
		ByteArrayToString(m_arOutPrefix.GetData(), m_arOutPrefix.GetSize(), stemp, _T("OutPrefix = \""));
		outfile << stemp << _T("\"\n");

		snprintf(strTemp, sizeof(strTemp), _T("[Log]\n"));
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("KeepLog = %s\n"), m_bKeepLog ? _T("1") : _T("0"));
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("LogComposedType = 0x%04X\n"), m_wLogComposedType);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("LogTypesToUnPack = \""));
		outfile << strTemp;
		if(m_bLogUnpackAll)
			outfile << _T("*");
		else
		{
			POSITION pos = m_mapLogMsgTypesToUnpack.GetStartPosition();
			while(pos != NULL)
			{
				WORD wType;
				void * pTemp;
				m_mapLogMsgTypesToUnpack.GetNextAssoc(pos, wType, pTemp);
				snprintf(strTemp, sizeof(strTemp), _T("0x%04X "), wType);
				outfile << strTemp;
			}
		}
		outfile << _T("\"\n");

		snprintf(strTemp, sizeof(strTemp), _T("LogComposedTypeToPack = 0x%04X\n"), m_wLogComposedTypeToPack);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("LogTypesToPack = \""));
		outfile << strTemp;
		if(m_bLogPackAll)
			outfile << _T("*");
		else
		{
			POSITION pos = m_mapLogMsgTypesToPack.GetStartPosition();
			while(pos != NULL)
			{
				WORD wType;
				void * pTemp;
				m_mapLogMsgTypesToPack.GetNextAssoc(pos, wType, pTemp);
				snprintf(strTemp, sizeof(strTemp), _T("0x%04X "), wType);
				outfile << strTemp;
			}
		}
		outfile << _T("\"\n");

		snprintf(strTemp, sizeof(strTemp), _T("[Status]\n"));
		outfile << strTemp;

		snprintf(strTemp, sizeof(strTemp), _T("SourceIndex = 0x%04X\n"), m_wSourceID);
		outfile << strTemp;
		snprintf(strTemp, sizeof(strTemp), _T("StatusRequestMessageType = 0x%04X\n"), m_wStatusRequestMessageType);
		outfile << strTemp;

		return TRUE;
	}
	catch(std::exception&)
	{
	}

	return FALSE;
}

void CSettings::MakeStatusMsg()
{
	m_StatusMsg.m_wMaxLength = 16 + (WORD)m_arStatusData.GetSize();
	m_StatusHdr.m_wMaxLength = 16 + m_StatusMsg.m_wMaxLength;
	m_arStatusMsg.SetSize(m_StatusHdr.m_wMaxLength);
	BYTE * pData = m_arStatusMsg.GetData();
	::ZeroMemory(pData, m_StatusHdr.m_wMaxLength);
	WORD * pHeader = (WORD *)pData;
	pHeader[0] = m_StatusHdr.m_wMaxLength;
	pHeader[1] = m_StatusHdr.m_wType;
	pHeader[2] = m_StatusHdr.m_wDestination;
	pHeader[3] = m_StatusHdr.m_wSource;
	pHeader[7] = m_StatusMsg.m_wMaxLength;
	pHeader[8] = m_StatusMsg.m_wType;
	pHeader[9] = m_StatusMsg.m_wDestination;
	pHeader[10] = m_StatusMsg.m_wSource;
	memcpy(pData + 28, m_arStatusData.GetData(), m_arStatusData.GetSize());
}

void CSettings::UpdateStatusMsg(unsigned char ind)
{
	BYTE * pData = m_arStatusMsg.GetData();
	UINT nLength = (UINT)m_arStatusMsg.GetSize();
	*((DWORD *)(pData + 8)) = *((DWORD *)(pData + 22)) = (DWORD)time(NULL);
	pData[12] = ind;
	pData[26] = ind;
	*((WORD *)(pData + nLength - sizeof(DWORD))) = CRC16(pData + 14, nLength - 14 - sizeof(DWORD), m_wCRC16Init);
	*((WORD *)(pData + nLength - sizeof(WORD))) = CRC16(pData, nLength - sizeof(WORD), m_wCRC16Init);
}
