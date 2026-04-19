// SettingView.cpp : implementation file
//
#include "pch.h"
#include "SettingView.h"
#include "resource.h"

namespace {

	const int kSettingProNameWidth = 260;

	const std::vector<std::wstring> kSettingPropName{
		L"GameProcessName",
		L"PacketDllName",
		L"GUIServerIP",
		L"GUIServerPort",
		L"IsTypeHeader1Byte",
		L"CInPacketFilterOpcodes (optional)",
		L"COutPacketFilterOpcodes (optional)",
		L"CInPacket::Decode1 Addr",
		L"CInPacket::Decode2 Addr",
		L"CInPacket::Decode4 Addr",
		L"CInPacket::Decode8 Addr (optional)",
		L"CInPacket::Skip8 Addr (optional)",
		L"CInPacket::DecodeStr Addr",
		L"CInPacket::DecodeBuffer Addr",
		L"COutPacket::Encode1 Addr",
		L"COutPacket::Encode2 Addr",
		L"COutPacket::Encode4 Addr",
		L"COutPacket::Encode8 Addr (optional)",
		L"COutPacket::EncodeStr Addr",
		L"COutPacket::EncodeBuffer Addr",
		L"COutPacket::MakeBufferList Addr",
		L"CClientSocket::ProcessPacket Addr",
		L"CClientSocket::SendPacket Addr (optional)",
		L"SendPacketEH Addr (optional)",
		L"CInPacket::Decode1 GenCode",
		L"CInPacket::Decode2 GenCode",
		L"CInPacket::Decode4 GenCode",
		L"CInPacket::Decode8 GenCode",
		L"CInPacket::Skip8 GenCode",
		L"CInPacket::DecodeStr GenCode",
		L"CInPacket::DecodeBufferGenCode",
		L"COutPacket::Encode1 GenCode",
		L"COutPacket::Encode2 GenCode",
		L"COutPacket::Encode4 GenCode",
		L"COutPacket::Encode8 GenCode",
		L"COutPacket::EncodeStr GenCode",
		L"COutPacket::EncodeBuffer GenCode",
	};

}
// SettingView dialog

IMPLEMENT_DYNAMIC(SettingView, CDialogEx)

SettingView::SettingView(IMainController* mainControllerImpl, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETTING_DIALOG, pParent), BaseView(L"SettingView")
{
	m_settingController = new SettingController(mainControllerImpl, this);
}

SettingView::~SettingView()
{
	delete(m_settingController);
	m_settingController = nullptr;
}

void SettingView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SETTING_MFCPROPERTYGRID, m_settingPropGridCtrl);
}

BOOL SettingView::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	this->SetHWND(this->GetSafeHwnd());
	// Set left column width
	HDITEM hdItem{};
	hdItem.mask = HDI_WIDTH;
	hdItem.cxy = kSettingProNameWidth;
	m_settingPropGridCtrl.GetHeaderCtrl().SetItem(0, &hdItem);
	// Add property
	const std::vector<std::wstring> settings = m_settingController->GetSettings();
	for (size_t i = 0; i < kSettingPropName.size(); i++) {
		std::wstring name = kSettingPropName[i];
		std::wstring value = settings[i];
		CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(name.c_str(), value.c_str());
		m_settingPropGridCtrl.AddProperty(pProp);
	}
	return TRUE;
}


BEGIN_MESSAGE_MAP(SettingView, CDialogEx)
	ON_BN_CLICKED(IDC_SAVE_SETTING_BUTTON, &SettingView::OnBnClickedSaveSettingButton)
	ON_BN_CLICKED(IDC_RESET_SETTING_BUTTON, &SettingView::OnBnClickedResetSettingButton)
END_MESSAGE_MAP()


// SettingView message handlers

void SettingView::OnBnClickedSaveSettingButton()
{
	// TODO: Add your control notification handler code here
	std::vector<std::wstring> propValues;

	int count = m_settingPropGridCtrl.GetPropertyCount();
	for (int i = 0; i < count; ++i)
	{
		CMFCPropertyGridProperty* pProp = m_settingPropGridCtrl.GetProperty(i);
		if (pProp != nullptr)
		{
			CString valueStr = pProp->GetValue();
			propValues.push_back(std::wstring(valueStr));
		}
	}
	bool ok = m_settingController->SetSetting(propValues);
	if (!ok) {
		MBError(L"Failed to save setting");
		return;
	}
	MBInfo(L"Save setting ok");
}

void SettingView::OnBnClickedResetSettingButton()
{
	// TODO: Add your control notification handler code here
	std::vector<std::wstring> propValues = m_settingController->ResetSetting();

	int count = m_settingPropGridCtrl.GetPropertyCount();
	for (int i = 0; i < count; ++i)
	{
		CMFCPropertyGridProperty* pProp = m_settingPropGridCtrl.GetProperty(i);
		if (pProp != nullptr)
		{
			pProp->SetValue(propValues[i].c_str());
		}
	}
}