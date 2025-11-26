//------------------------------------------------------------------------------
// import_kmf.hpp
//
// Copyright (C) 2025 Lost Empire Entertainment
//
// This is free source code, and you are welcome to redistribute it under certain conditions.
// Read LICENSE.md for more information.
//
// Provides:
//   - Helpers for streaming individual models or loading the full kalamodelfile binary into memory
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------

# KMF binary top header for model export-import

Offset | Size | Field
-------|------|--------------------------------------------
0      | 4    | KMF magic word, always 'K', 'M', 'F', '\0' aka '0x00464D4B'
4      | 1    | kmf binary version
5      | 4    | max number of allowed models, max is 1024
N      | 4    | model table size in bytes max is N KB
N      | 4    | model block size in bytes, max is 1024 KB

# KMF binary model table for model export-import

Offset | Size | Field
-------|------|--------------------------------------------

??+4   | 4    | absolute offset from start of file relative to its model block start
??+8   | 4    | size of the glyh block (info + payload)

# KMF binary model block for model export-import

Offset | Size | Field
-------|------|--------------------------------------------

...

------------------------------------------------------------------------------*/

#pragma once

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <filesystem>

namespace KalaHeaders
{
	using std::vector;
	using std::array;
	using std::string;
	using std::ifstream;
	using std::filesystem::path;
	using std::filesystem::current_path;
	using std::filesystem::weakly_canonical;
	using std::filesystem::exists;
	using std::filesystem::is_regular_file;
	using std::filesystem::perms;
	using std::filesystem::status;
	using std::streamoff;
	using std::streamsize;
	using std::ios;
	using std::move;
	
	using u8 = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using i8 = int8_t;
	using i16 = int16_t;
	
	//The magic that must exist in all kmf files at the first four bytes
	constexpr u32 KMF_MAGIC = 0x0046544B;
	
	//The version that must exist in all kmf files as the fifth byte
	constexpr u8 KMF_VERSION = 1;
	
	//The true top header size that is always required
	constexpr u8 CORRECT_MODEL_HEADER_SIZE = 34u;
	
	//The true per-model table size that is always required
	constexpr u8 CORRECT_MODEL_TABLE_SIZE = 12u;
	
	//Max allowed models for exporting
	constexpr u16 MAX_MODEL_COUNT = 1024u;
	
	//Max allowed total model table size in bytes for bitmap and model exporting (12 KB)
	constexpr u32 MAX_MODEL_TABLE_SIZE = 12288u;
	
	//Max allowed total model blocks size in bytes for exporting (1024 KB)
	constexpr u32 MAX_MODEL_BLOCK_SIZE = 1048576u;
	
	constexpr u32 MIN_TOTAL_SIZE = 
		CORRECT_MODEL_HEADER_SIZE
		+ CORRECT_MODEL_TABLE_SIZE;
	
	//Max allowed size for kmf files
	constexpr u32 MAX_TOTAL_SIZE = 
		CORRECT_MODEL_HEADER_SIZE 
		+ MAX_MODEL_TABLE_SIZE 
		+ MAX_MODEL_BLOCK_SIZE;
	
	//The main header at the top of each kmf file
	struct ModelHeader
	{
		
	};

	//The table that helps look up models individually
	struct ModelTable
	{
		
	};
		
	//The block containing data of each model
	struct ModelBlock
	{
		
	};
	
	enum class ImportResult : u8
	{
		RESULT_SUCCESS                     = 0, //No errors, succeeded with import
		
		//
		// FILE OPERATIONS
		//
		
		RESULT_FILE_NOT_FOUND              = 1,  //File does not exist
		RESULT_INVALID_EXTENSION           = 2,  //File is not '.kmf'
		RESULT_UNAUTHORIZED_READ           = 3,  //Not authorized to read this file
		RESULT_FILE_LOCKED                 = 4,  //Cannot read this file, file is in use
		RESULT_UNKNOWN_READ_ERROR          = 5,  //Unknown file error when reading file
		RESULT_FILE_EMPTY                  = 6,  //There is no content inside this file
		
		//
		// IMPORT ERRORS
		//
		
		RESULT_UNSUPPORTED_FILE_SIZE       = 7,  //Always assume total size is atleast 52 bytes
		
		RESULT_INVALID_MAGIC               = 8,  //magic must be 'KMF\0'
		RESULT_INVALID_VERSION             = 9,  //version must match
		RESULT_INVALID_MODEL_HEADER_SIZE   = 11, //found a model header that wasnt the correct size
		RESULT_INVALID_MODEL_TABLE_SIZE    = 12, //found a model table that wasnt the correct size
		RESULT_INVALID_MODEL_BLOCK_SIZE    = 13, //found a model block that was less or more than the allowed size
		RESULT_INVALID_MODEL_COUNT         = 14, //total model count was above allowed max model count
		RESULT_UNEXPECTED_EOF              = 15  //file reached end sooner than expected
	};
	
	inline string ResultToString(ImportResult result)
	{
		switch (result)
		{
		default: return "RESULT_UNKNOWN";
			
		case ImportResult::RESULT_SUCCESS:
			return "RESULT_SUCCESS";
		
		case ImportResult::RESULT_FILE_NOT_FOUND:
			return "RESULT_FILE_NOT_FOUND";
		case ImportResult::RESULT_INVALID_EXTENSION:
			return "RESULT_INVALID_EXTENSION";
		case ImportResult::RESULT_UNAUTHORIZED_READ:
			return "RESULT_UNAUTHORIZED_READ";
		case ImportResult::RESULT_FILE_LOCKED:
			return "RESULT_FILE_LOCKED";
		case ImportResult::RESULT_UNKNOWN_READ_ERROR:
			return "RESULT_UNKNOWN_READ_ERROR";
		case ImportResult::RESULT_FILE_EMPTY:
			return "RESULT_FILE_EMPTY";
			
		case ImportResult::RESULT_UNSUPPORTED_FILE_SIZE:
			return "RESULT_UNSUPPORTED_FILE_SIZE";
			
		case ImportResult::RESULT_INVALID_MAGIC:
			return "RESULT_INVALID_MAGIC";
		case ImportResult::RESULT_INVALID_VERSION:
			return "RESULT_INVALID_VERSION";
		case ImportResult::RESULT_INVALID_MODEL_HEADER_SIZE:
			return "RESULT_INVALID_MODEL_HEADER_SIZE";
		case ImportResult::RESULT_INVALID_MODEL_TABLE_SIZE:
			return "RESULT_INVALID_MODEL_TABLE_SIZE";
		case ImportResult::RESULT_INVALID_MODEL_BLOCK_SIZE:
			return "RESULT_INVALID_MODEL_BLOCK_SIZE";
		case ImportResult::RESULT_INVALID_MODEL_COUNT:
			return "RESULT_INVALID_MODEL_COUNT";
		case ImportResult::RESULT_UNEXPECTED_EOF:
			return "RESULT_UNEXPECTED_EOF";
		}
		
		return "RESULT_UNKNOWN";
	}
	
	//Takes in a path to the .kmf file and returns binary data with a result enum
	inline ImportResult ImportKMF(
		const path& inFile,
		ModelHeader& outHeader,
		vector<ModelTable>& outTables,
		vector<ModelBlock>& outBlocks)
	{
		//
		// PRE-READ CHECKS
		//
		
		if (!exists(inFile)) return ImportResult::RESULT_FILE_NOT_FOUND;
		if (!is_regular_file(inFile)
			|| !inFile.has_extension()
			|| inFile.extension() != ".kmf")
		{
			return ImportResult::RESULT_INVALID_EXTENSION;
		}
		
		auto fileStatus = status(inFile);
		auto filePerms = fileStatus.permissions();
		
		bool canRead = (filePerms & (
			perms::owner_read
			| perms::group_read
			| perms::others_read))  
			!= perms::none;
		
		if (!canRead) return ImportResult::RESULT_UNAUTHORIZED_READ;
				
		try
		{
			//
			// TRY TO OPEN AND READ
			//
			
			errno = 0;
			ifstream in(inFile, ios::in | ios::binary);
			if (in.fail()
				&& errno != 0)
			{
				if (errno == EBUSY
					|| errno == ETXTBSY)
				{
					return ImportResult::RESULT_FILE_LOCKED;
				}
				else return ImportResult::RESULT_UNKNOWN_READ_ERROR;
			}
			
			in.seekg(0, ios::end);
			size_t fileSize = static_cast<size_t>(in.tellg());
			
			if (fileSize == 0) return ImportResult::RESULT_FILE_EMPTY;
			if (fileSize < MIN_TOTAL_SIZE)
			{
				return ImportResult::RESULT_UNSUPPORTED_FILE_SIZE;
			}
			if (fileSize > MAX_TOTAL_SIZE)
			{
				return ImportResult::RESULT_UNSUPPORTED_FILE_SIZE;
			}
			
			in.seekg(static_cast<streamoff>(0), ios::beg);
			
			//
			// PARSE FOUND DATA
			//
			
			vector<u8> rawData(fileSize);
			
			in.read(
				reinterpret_cast<char*>(rawData.data()),
				static_cast<streamsize>(fileSize));
				
			in.close();	
				
			ModelHeader header{};
			
			//model header
			
			//model table data
			
			//model block data
		}
		catch (...)
		{
			return ImportResult::RESULT_UNKNOWN_READ_ERROR;
		}
		
		return ImportResult::RESULT_SUCCESS;
	}
}