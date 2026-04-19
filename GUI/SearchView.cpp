// SearchView.cpp : implementation file
//
#include "pch.h"
#include "SearchView.h"
#include "ColumnStyle.h"
#include "resource.h"

// SearchView dialog

IMPLEMENT_DYNAMIC(SearchView, CDialogEx)

SearchView::SearchView(IMainController* mainControllerImpl, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SEARCH_DIALOG, pParent), BaseView(L"SearchView")
{
	m_searchController = new SearchController(mainControllerImpl, this);
}

SearchView::~SearchView()
{
	delete m_searchController;
	m_searchController = nullptr;
}

void SearchView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SEARCH_CONDITION_COMBO, m_searchConditionComboBox);
	DDX_Control(pDX, IDC_SEARCH_PROCESS_ID_EDIT, m_searchPIDEdit);
	DDX_Control(pDX, IDC_SEARCH_TEXT_EDIT, m_searchTextEdit);
	DDX_Control(pDX, IDC_PACKET_LOG_SEARCH_LIST, m_packetLogSearchListCtrl);
}

BOOL SearchView::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	this->SetHWND(this->GetSafeHwnd());
	// Set search condition
	const std::vector<std::wstring> conditions = m_searchController->GetSearchConditionText();
	for (const std::wstring& condition : conditions) {
		m_searchConditionComboBox.AddString(condition.c_str());
	}
	m_searchConditionComboBox.SetCurSel(0);
	// Set "PID" as default value
	m_searchPIDEdit.SendMessageW(EM_SETCUEBANNER, 0, (LPARAM)_T("PID"));
	// Set extended styles:
	// - LVS_EX_FULLROWSELECT: Selecting any column highlights the row
	// - LVS_EX_GRIDLINES: Displays grid lines between rows and columns for better readability
	m_packetLogSearchListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	// Set column title and width
	CRect dlgClientRect;
	GetClientRect(&dlgClientRect);
	int totalWidth = dlgClientRect.Width();
	std::vector<int> columnWidths = GetColumnWidths(totalWidth, kLogColumnWidthRatio);
	for (size_t i = 0; i < columnWidths.size(); i++)
	{
		m_packetLogSearchListCtrl.InsertColumn(static_cast<int>(i), kLogColumnTitle[i].c_str(), LVCFMT_LEFT, columnWidths[i]);
	}
	return TRUE;
}

void SearchView::PostNcDestroy()
{
	delete this;
}


BEGIN_MESSAGE_MAP(SearchView, CDialogEx)
	ON_BN_CLICKED(IDC_FIND_BUTTON, &SearchView::OnBnClickedFindButton)
	ON_NOTIFY(NM_DBLCLK, IDC_PACKET_LOG_SEARCH_LIST, &SearchView::OnNMDblclkPacketLogList)
END_MESSAGE_MAP()


// SearchView message handlers

BOOL SearchView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN) {
			// Avoid the dialog is closed by enter key
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void SearchView::OnBnClickedFindButton()
{
	// TODO: Add your control notification handler code here
	int selIndex = m_searchConditionComboBox.GetCurSel();
	CString pidInput;
	CString searchInput;
	m_searchPIDEdit.GetWindowTextW(pidInput);
	m_searchTextEdit.GetWindowTextW(searchInput);
	int pid = _wtoi(pidInput);
	std::wstring searchText(searchInput);
	m_packetLogSearchListCtrl.DeleteAllItems();
	std::vector<std::pair<int, PacketLogModel>> result = m_searchController->SearchPacketLog(selIndex, pid, searchText);
	for (size_t i = 0; i < result.size(); i++)
	{
		int n = static_cast<int>(i);
		const std::wstring id = std::to_wstring(result[n].first);
		const PacketLogModel& log = result[n].second;
		m_packetLogSearchListCtrl.InsertItem(n, id.c_str());
		m_packetLogSearchListCtrl.SetItemText(n, static_cast<int>(kLogColumnType::PID), log.GetPIDStr().c_str());
		m_packetLogSearchListCtrl.SetItemText(n, static_cast<int>(kLogColumnType::Index), log.GetIndexStr().c_str());
		m_packetLogSearchListCtrl.SetItemText(n, static_cast<int>(kLogColumnType::Type), log.GetType().c_str());
		m_packetLogSearchListCtrl.SetItemText(n, static_cast<int>(kLogColumnType::Length), log.GetLengthStr().c_str());
		m_packetLogSearchListCtrl.SetItemText(n, static_cast<int>(kLogColumnType::Opcode), log.GetOpcodeStr().c_str());
		m_packetLogSearchListCtrl.SetItemText(n, static_cast<int>(kLogColumnType::Data), log.GetData().c_str());
	}
}

void SearchView::OnNMDblclkPacketLogList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	int nSelectedIndex = pNMItemActivate->iItem;
	if (nSelectedIndex == -1) {
		MBError(L"Please select one row");
		return;
	}
	int nSelectedID = _wtoi(m_packetLogSearchListCtrl.GetItemText(nSelectedIndex, 0));
	m_searchController->JumpLogItem((nSelectedID));
	*pResult = 0;
}