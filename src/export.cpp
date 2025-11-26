//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <fstream>
#include <string>

#include "KalaHeaders/log_utils.hpp"
#include "KalaHeaders/file_utils.hpp"
#include "KalaHeaders/import_kmf.hpp"

#include "export.hpp"

using KalaHeaders::Log;
using KalaHeaders::LogType;
using KalaHeaders::WriteU8;
using KalaHeaders::WriteU16;
using KalaHeaders::WriteU32;
using KalaHeaders::WriteI8;
using KalaHeaders::WriteI16;
using KalaHeaders::WriteI32;
using KalaHeaders::ModelHeader;
using KalaHeaders::CORRECT_MODEL_HEADER_SIZE;
using KalaHeaders::CORRECT_MODEL_TABLE_SIZE;
using KalaHeaders::MAX_MODEL_COUNT;
using KalaHeaders::MAX_MODEL_TABLE_SIZE;

using std::ofstream;
using std::ios;
using std::to_string;

using u8 = uint8_t;
using i8 = int8_t;
using i16 = int16_t;
using u32 = uint32_t;

namespace KalaModel
{
	void Export::ExportKMF(
		const path& targetPath,
		vector<ModelBlock>& modelBlocks)
	{
		if (modelBlocks.size() > MAX_MODEL_COUNT)
		{
			Log::Print(
				"Failed to export because model count exceeded max allowed count '" + to_string(MAX_MODEL_COUNT) + "'!",
				"EXPORT_MODEL",
				LogType::LOG_ERROR,
				2);
		
			return;
		}
		
		if (CORRECT_MODEL_TABLE_SIZE * modelBlocks.size() > MAX_MODEL_TABLE_SIZE)
		{
			Log::Print(
				"Failed to export because model data size exceeded max allowed size '" + to_string(MAX_MODEL_TABLE_SIZE) + "'!",
				"EXPORT_MODEL",
				LogType::LOG_ERROR,
				2);
		
			return;
		}
		
		Log::Print(
			"Starting to export models to path '" + targetPath.string() + "'.",
			"EXPORT_MODEL",
			LogType::LOG_DEBUG);
			
		vector<u8> output{};
		vector<u8> modelTableOutput{};
		vector<u8> modelBlockOutput{};
		
		//
		// FIRST STORE THE TOP HEADER
		//
			
		
	}
}