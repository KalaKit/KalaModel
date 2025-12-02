//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <fstream>
#include <string>

#include "KalaHeaders/log_utils.hpp"
#include "KalaHeaders/file_utils.hpp"
#include "KalaHeaders/import_kmd.hpp"

#include "export.hpp"

using KalaHeaders::Log;
using KalaHeaders::LogType;
using KalaHeaders::WriteU8;
using KalaHeaders::WriteU16;
using KalaHeaders::WriteU32;
using KalaHeaders::WriteFixedString;
using KalaHeaders::KalaModelData::ModelHeader;
using KalaHeaders::KalaModelData::Vertex;
using KalaHeaders::KalaModelData::CORRECT_MODEL_HEADER_SIZE;
using KalaHeaders::KalaModelData::CORRECT_MODEL_TABLE_SIZE;
using KalaHeaders::KalaModelData::VERTICE_DATA_OFFSET;
using KalaHeaders::KalaModelData::MAX_MODEL_COUNT;
using KalaHeaders::KalaModelData::MAX_MODEL_TABLE_SIZE;

using std::ofstream;
using std::ios;
using std::to_string;
using std::bit_cast;

using u8 = uint8_t;
using u32 = uint32_t;
using f32 = float;

namespace KalaModel
{
	void Export::ExportKMF(
		const path& targetPath,
		u8 scaleFactor,
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
			
		ModelHeader modelHeader{};
		
		u32 offset{};
		
		output.reserve(CORRECT_MODEL_HEADER_SIZE);
		
		WriteU32(output, offset, modelHeader.magic);  offset += 4;
		WriteU8(output, offset, modelHeader.version); offset++;
		WriteU8(output, offset, scaleFactor);         offset++;
		WriteU32(output, offset, modelBlocks.size()); offset += 4;
		
		//
		// THEN STORE THE MODEL TABLES
		//
		
		size_t totalMTBytes = CORRECT_MODEL_TABLE_SIZE * modelBlocks.size();
		modelTableOutput.reserve(totalMTBytes);
		
		u32 baseOffset = CORRECT_MODEL_HEADER_SIZE + totalMTBytes;
		u32 tableOffset{};
		
		for (const auto& b : modelBlocks)
		{
			u32 blockSize = VERTICE_DATA_OFFSET + b.verticesSize + b.indicesSize;
			
			WriteFixedString(
				modelTableOutput,
				tableOffset,
				b.nodeName,
				sizeof(b.nodeName));
				
			WriteU32(modelTableOutput, tableOffset + 20, baseOffset);
			WriteU32(modelTableOutput, tableOffset + 24, blockSize);
			
			//next table entry (internal buffer)
			tableOffset += CORRECT_MODEL_TABLE_SIZE;
			
			//next model block (absolute in final file)
			baseOffset += blockSize;
		}
		
		//
		// THEN STORE THE MODEL BLOCKS
		//
		
		size_t totalMBBytes{};
		for (const auto& b : modelBlocks) totalMBBytes += VERTICE_DATA_OFFSET + b.verticesSize + b.indicesSize;
		
		modelBlockOutput.reserve(totalMBBytes);
		
		u32 mOffset{};
		
		for (const auto& m : modelBlocks)
		{
			WriteFixedString(
				modelBlockOutput,
				mOffset,
				m.nodeName,
				sizeof(m.nodeName));
			mOffset += 20;
			
			WriteFixedString(
				modelBlockOutput,
				mOffset,
				m.meshName,
				sizeof(m.meshName));
			mOffset += 20;
			
			WriteFixedString(
				modelBlockOutput,
				mOffset,
				m.nodePath,
				sizeof(m.nodePath));
			mOffset += 50;
				
			WriteU8(modelBlockOutput, mOffset, m.dataTypeFlags); mOffset++;
			WriteU8(modelBlockOutput, mOffset, m.renderType);    mOffset++;
			
			WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(m.position[0])); mOffset += 4;
			WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(m.position[1])); mOffset += 4;
			WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(m.position[2])); mOffset += 4;
			
			WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(m.rotation[0])); mOffset += 4;
			WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(m.rotation[1])); mOffset += 4;
			WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(m.rotation[2])); mOffset += 4;
			WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(m.rotation[3])); mOffset += 4;
			
			WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(m.size[0])); mOffset += 4;
			WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(m.size[1])); mOffset += 4;
			WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(m.size[2])); mOffset += 4;
			
			WriteU32(modelBlockOutput, mOffset, m.verticesOffset); mOffset += 4;
			WriteU32(modelBlockOutput, mOffset, m.verticesSize);   mOffset += 4;
			WriteU32(modelBlockOutput, mOffset, m.indicesOffset);  mOffset += 4;
			WriteU32(modelBlockOutput, mOffset, m.indicesSize);    mOffset += 4;
			
			for (const auto& v : m.vertices)
			{
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(v.position[0])); mOffset += 4;
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(v.position[1])); mOffset += 4;
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(v.position[2])); mOffset += 4;
				
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(v.normal[0])); mOffset += 4;
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(v.normal[1])); mOffset += 4;
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(v.normal[2])); mOffset += 4;
				
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(v.texCoord[0])); mOffset += 4;
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(v.texCoord[1])); mOffset += 4;
				
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(v.tangent[0])); mOffset += 4;
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(v.tangent[1])); mOffset += 4;
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(v.tangent[2])); mOffset += 4;
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(v.tangent[3])); mOffset += 4;
			}
			
			for (int i = 0; i < m.indices.size(); i++)
			{
				WriteU32(modelBlockOutput, mOffset, bit_cast<u32>(m.indices[i]));
				mOffset += 4;
			}
		}
		
		//
		// AND PASS THE FINAL DATA
		//
		
		WriteU32(output, offset, totalMTBytes); offset += 4;
		WriteU32(output, offset, totalMBBytes); offset += 4;
		
		output.reserve(CORRECT_MODEL_HEADER_SIZE + totalMTBytes + totalMBBytes);
		
		output.insert(output.end(), modelTableOutput.begin(), modelTableOutput.end());
		output.insert(output.end(), modelBlockOutput.begin(), modelBlockOutput.end());
			
		ofstream file(
			targetPath,
			ios::binary);
			
		file.write(
			reinterpret_cast<const char*>(output.data()), output.size());
			
		file.close();
			
		Log::Print(
			"Finished exporting models!",
			"EXPORT_MODEL",
			LogType::LOG_SUCCESS);
	}
}