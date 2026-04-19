// FormatView.cpp : implementation file
//
#include "pch.h"
#include "FormatView.h"
#include "resource.h"
#include "ColumnStyle.h"

namespace {
	const COLORREF kEditableCellColor = RGB(220, 255, 220);
}

// FormatView dialog

IMPLEMENT_DYNAMIC(FormatView, CDialogEx)

FormatView::FormatView(int nSelectedID, IMainController* mainControllerImpl, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FORMAT_DIALOG, pParent), BaseView(L"FormatView")
{
	m_formatController = new FormatController(nSelectedID, mainControllerImpl, this);
}

FormatView::~FormatView()
{
	delete(m_formatController);
	delete(m_genCodeView);
	m_formatController = nullptr;
	m_genCodeView = nullptr;
}

void FormatView::UpdateCellWidth()
{
	CRect dlgClientRect;
	GetClientRect(&dlgClientRect);
	CRect gridMoveRect(dlgClientRect.left, dlgClientRect.top, dlgClientRect.Width(), dlgClientRect.Height() - 50);
	m_packetFormatGridCtrl.MoveWindow(gridMoveRect, TRUE);
	CRect gridClientRect;
	m_packetFormatGridCtrl.GetClientRect(&gridClientRect);
	int totalWidth = gridClientRect.Width();
	std::vector<int> columnWidths = GetColumnWidths(totalWidth, kFormatColumnWidthRatio);
	for (size_t i = 0; i < columnWidths.size(); i++)
	{
		m_packetFormatGridCtrl.SetColumnWidth(
			static_cast<int>(i),
			static_cast<int>(columnWidths[i])
		);
	}
}

void FormatView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BOOL FormatView::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	this->SetHWND(this->GetSafeHwnd());
	// Set dialog title
	this->SetDialogTitleWithID(m_formatController->GetLogID());
	// Adjust grid size
	m_packetFormatGridCtrl.Create(CRect{}, this, IDC_PACKET_FORMAT_GRID, WS_CHILD | WS_VISIBLE | WS_BORDER);
	m_packetFormatGridCtrl.SetFixedRowCount(1);
	m_packetFormatGridCtrl.SetColumnCount(static_cast<int>(kFormatColumnTitle.size()));
	m_genCodeButtonAnchorInfo = this->GetAnchorInfo(GetDlgItem(IDC_GEN_CODE_BUTTON));
	m_sendButtonAnchorInfo = this->GetAnchorInfo(GetDlgItem(IDC_FORMAT_SEND_BUTTON));
	// Set column title
	for (size_t i = 0; i < kFormatColumnTitle.size(); i++)
	{
		std::wstring title = kFormatColumnTitle[i];
		m_packetFormatGridCtrl.SetItemText(0, static_cast<int>(i), title.c_str());
	}
	// Adjust grid rect size
	this->UpdateCellWidth();
	// Draw grid data
	const std::vector<PacketFormatModel> pFormats = m_formatController->GetPacketFormatModels();
	m_packetFormatGridCtrl.SetRowCount(static_cast<int>(pFormats.size() + 1));
	for (size_t i = 0; i < pFormats.size(); i++)
	{
		// Set cell text
		const PacketFormatModel pFormat = pFormats[i];
		int currentRow = static_cast<int>(i + 1);
		m_packetFormatGridCtrl.SetItemText(currentRow, static_cast<int>(kFormatColumnType::Index), std::to_wstring(pFormat.GetIndex()).c_str());
		m_packetFormatGridCtrl.SetItemText(currentRow, static_cast<int>(kFormatColumnType::RetAddr), pFormat.GetRetAddr().c_str());
		m_packetFormatGridCtrl.SetItemText(currentRow, static_cast<int>(kFormatColumnType::Action), pFormat.GetActionText().c_str());
		m_packetFormatGridCtrl.SetItemText(currentRow, static_cast<int>(kFormatColumnType::Size), std::to_wstring(pFormat.GetSize()).c_str());
		m_packetFormatGridCtrl.SetItemText(currentRow, static_cast<int>(kFormatColumnType::Value), pFormat.GetValue().c_str());
		m_packetFormatGridCtrl.SetItemText(currentRow, static_cast<int>(kFormatColumnType::Segment), pFormat.GetSegment().c_str());
		m_packetFormatGridCtrl.SetItemText(currentRow, static_cast<int>(kFormatColumnType::Comment), pFormat.GetComment().c_str());
		// Set column state
		m_packetFormatGridCtrl.SetItemState(currentRow, static_cast<int>(kFormatColumnType::Index), GVIS_READONLY);
		m_packetFormatGridCtrl.SetItemState(currentRow, static_cast<int>(kFormatColumnType::RetAddr), GVIS_READONLY);
		m_packetFormatGridCtrl.SetItemState(currentRow, static_cast<int>(kFormatColumnType::Action), GVIS_READONLY);
		m_packetFormatGridCtrl.SetItemState(currentRow, static_cast<int>(kFormatColumnType::Size), GVIS_READONLY);
		// Set editable cell color
		m_packetFormatGridCtrl.SetItemBkColour(currentRow, static_cast<int>(kFormatColumnType::Value), kEditableCellColor);
		m_packetFormatGridCtrl.SetItemBkColour(currentRow, static_cast<int>(kFormatColumnType::Segment), kEditableCellColor);
		m_packetFormatGridCtrl.SetItemBkColour(currentRow, static_cast<int>(kFormatColumnType::Comment), kEditableCellColor);
	}
	return TRUE;
}

void FormatView::PostNcDestroy()
{
	delete this;
}


BEGIN_MESSAGE_MAP(FormatView, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(GVN_BEGINLABELEDIT, IDC_PACKET_FORMAT_GRID, &FormatView::OnGridCellBeginEdit)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_PACKET_FORMAT_GRID, &FormatView::OnGridCellEdited)
	ON_BN_CLICKED(IDC_GEN_CODE_BUTTON, &FormatView::OnBnClickedGenCodeButton)
	ON_BN_CLICKED(IDC_FORMAT_SEND_BUTTON, &FormatView::OnBnClickedSendButton)
END_MESSAGE_MAP()

BOOL FormatView::PreTranslateMessage(MSG* pMsg)
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

void FormatView::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_packetFormatGridCtrl.GetSafeHwnd())
	{
		this->UpdateCellWidth();
		this->UpdateCtrlPosition(GetDlgItem(IDC_GEN_CODE_BUTTON), cx, cy, m_genCodeButtonAnchorInfo);
		this->UpdateCtrlPosition(GetDlgItem(IDC_FORMAT_SEND_BUTTON), cx, cy, m_sendButtonAnchorInfo);
	}
}

void FormatView::OnGridCellBeginEdit(NMHDR* pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;
	int row = pItem->iRow;
	int col = pItem->iColumn;
	m_prevCellText = m_packetFormatGridCtrl.GetItemText(row, col);
	*pResult = 0;
}

void FormatView::OnGridCellEdited(NMHDR* pNotifyStruct, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;
	int row = pItem->iRow;
	int col = pItem->iColumn;
	std::wstring text = m_packetFormatGridCtrl.GetItemText(row, col).GetString();
	// Value->Segment
	if (col == static_cast<int>(kFormatColumnType::Value)) {
		std::wstring segment = L"";
		std::wstring strLen = L"";
		bool ok = m_formatController->EncodeValue(row, text, segment, strLen);
		if (!ok) {
			m_packetFormatGridCtrl.SetItemText(row, col, m_prevCellText);
			return;
		}
		if (!strLen.empty()) {
			m_packetFormatGridCtrl.SetItemText(row, static_cast<int>(kFormatColumnType::Size), strLen.c_str());
			m_packetFormatGridCtrl.RedrawCell(row, static_cast<int>(kFormatColumnType::Size));
		}
		m_packetFormatGridCtrl.SetItemText(row, static_cast<int>(kFormatColumnType::Segment), segment.c_str());
		m_packetFormatGridCtrl.RedrawCell(row, static_cast<int>(kFormatColumnType::Segment));
	}
	// Segment -> Value
	if (col == static_cast<int>(kFormatColumnType::Segment)) {
		std::wstring value = L"";
		std::wstring strLen = L"";
		bool ok = m_formatController->DecodeSegment(row, text, value, strLen);
		if (!ok) {
			m_packetFormatGridCtrl.SetItemText(row, col, m_prevCellText);
			return;
		}
		if (!strLen.empty()) {
			m_packetFormatGridCtrl.SetItemText(row, static_cast<int>(kFormatColumnType::Size), strLen.c_str());
			m_packetFormatGridCtrl.RedrawCell(row, static_cast<int>(kFormatColumnType::Size));
		}
		m_packetFormatGridCtrl.SetItemText(row, static_cast<int>(kFormatColumnType::Value), value.c_str());
		m_packetFormatGridCtrl.RedrawCell(row, static_cast<int>(kFormatColumnType::Value));
	}
	// Update comment
	if (col == static_cast<int>(kFormatColumnType::Comment)) {
		bool ok = m_formatController->UpdateCommnet(row, text);
		if (!ok) {
			m_packetFormatGridCtrl.SetItemText(row, col, m_prevCellText);
			return;
		}
	}

}

void FormatView::OnBnClickedGenCodeButton()
{
	// TODO: Add your control notification handler code here
	GenCodeView* genCodeView = new GenCodeView(m_formatController, this);
	genCodeView->Create(IDD_GEN_CODE_DIALOG, this);
	genCodeView->ShowWindow(SW_SHOW);
}


void FormatView::OnBnClickedSendButton()
{
	// TODO: Add your control notification handler code here
	std::wstring err = m_formatController->SendFormatData();
	if (!err.empty()) {
		MBError(err);
		return;
	}
	MBInfo(L"Send data ok");
}