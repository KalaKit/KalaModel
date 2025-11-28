//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <vector>
#include <filesystem>

#include "KalaHeaders/import_kmd.hpp"

using KalaHeaders::ModelBlock;

namespace KalaModel
{
	using std::vector;
	using std::filesystem::path;
	
	using u8 = uint8_t;
	
	class Export
	{
	public:
		//Export as kmf
		static void ExportKMF(
			const path& targetPath,
			u8 scaleFactor,
			vector<ModelBlock>& modelBlocks);
	};
}