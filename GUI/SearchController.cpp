#include "pch.h"
#include "SearchView.h"
#include "SearchController.h"

namespace {
	const std::vector<std::wstring> kSearchConditionText = {
		L"Opcode",
		L"In",
		L"Out",
		L"Data",
		L"All",
	};

	const enum kSearchCondition {
		Opcode,
		In,
		Out,
		Data,
		All,
	};

	static std::wstring searchText2Regex(const std::wstring& data) {
		if (data.empty()) {
			return L"";
		}
		std::wistringstream wiss(data);
		std::wstring token;
		std::wstring regexBody;

		while (wiss >> token) {
			if (token == L"??") {
				regexBody += LR"(\S+)";
			}
			else {
				regexBody += token;
			}
			regexBody += LR"(\s+)";
		}

		// Remove the last space
		if (!regexBody.empty()) {
			regexBody.erase(regexBody.size() - 3);
		}

		return regexBody;

	}

}

SearchController::SearchController(IMainController* mainControllerImpl, SearchView* view)
{
	m_mainControllerImpl = mainControllerImpl;
	m_searchView = view;
}

SearchController::~SearchController()
{
}

const std::vector<std::wstring> SearchController::GetSearchConditionText()
{
	return kSearchConditionText;
}

std::vector<std::pair<int, PacketLogModel>> SearchController::SearchPacketLog(int condition, int pid, const std::wstring& searchText)
{
	std::vector<std::pair<int, PacketLogModel>> result;
	const std::vector<PacketLogModel> packetLogModels = m_mainControllerImpl->GetPacketLogModels();
	for (size_t i = 0; i < packetLogModels.size(); i++)
	{
		const PacketLogModel log = packetLogModels[i];
		if (pid > 0 && pid != log.GetPID()) {
			continue;
		}
		switch (condition) {
		case kSearchCondition::Opcode: {
			if (searchText == log.GetOpcodeStr()) {
				result.emplace_back(i, log);
			}
			break;
		}
		case kSearchCondition::In: {
			if (log.GetType() == L"In") {
				result.emplace_back(i, log);
			}
			break;
		}
		case kSearchCondition::Out: {
			if (log.GetType() == L"Out") {
				result.emplace_back(i, log);
			}
			break;
		}
		case kSearchCondition::Data: {
			std::wstring regexStr = searchText2Regex(searchText);
			if (!regexStr.empty()) {
				try {
					std::wregex pattern(regexStr);
					if (std::regex_search(log.GetData(), pattern)) {
						result.emplace_back(i, log);
					}
				}
				catch (const std::regex_error& e) {
					std::wstring err = PacketScript::MultiByte2WideChar(e.what(), strlen(e.what()));
					this->m_searchView->MBError(err + L"\r\n" + regexStr);
				}
			}
			break;
		}
		case kSearchCondition::All: {
			result.emplace_back(i, log);
			break;
		}
		}

	}
	return result;
}


void SearchController::JumpLogItem(int logID) {
	bool ok = m_mainControllerImpl->JumpLogItem(logID);
	if (!ok) {
		m_searchView->MBError(L"Failed to jump to log item");
	}
}
