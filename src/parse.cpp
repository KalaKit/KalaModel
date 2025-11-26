//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <vector>
#include <string>
#include <filesystem>
#include <sstream>

#include "KalaHeaders/log_utils.hpp"
#include "KalaHeaders/string_utils.hpp"
#include "KalaHeaders/import_kmf.hpp"

#include "KalaCLI/include/core.hpp"

#include "parse.hpp"
#include "export.hpp"

using KalaHeaders::Log;
using KalaHeaders::LogType;
using KalaHeaders::HasAnyNonNumber;
using KalaHeaders::HasAnyWhiteSpace;
using KalaHeaders::ModelBlock;

using KalaCLI::Core;

using KalaModel::Export;

using std::vector;
using std::string;
using std::to_string;
using std::filesystem::current_path;
using std::filesystem::path;
using std::filesystem::weakly_canonical;
using std::filesystem::is_regular_file;
using std::filesystem::status;
using std::filesystem::perms;
using std::ostringstream;
using std::hex;
using std::dec;
using std::move;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using i16 = int16_t;

static void ParseAny(
	const vector<string>& params,
	bool isVerbose);

namespace KalaModel
{
	void Parse::Command_Parse(const vector<string>& params)
	{
		ParseAny(params, false);
	}
	
	void Parse::Command_VerboseParse(const vector<string>& params)
	{
		ParseAny(params, true);
	}
}

void ParseAny(
	const vector<string>& params,
	bool isVerbose)
{
	
}