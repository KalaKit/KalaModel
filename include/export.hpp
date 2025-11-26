//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <vector>
#include <filesystem>

#include "KalaHeaders/import_kmf.hpp"

using KalaHeaders::ModelBlock;

namespace KalaModel
{
	using std::vector;
	using std::filesystem::path;
	
	class Export
	{
	public:
		//Export as kmf
		static void ExportKMF(
			const path& targetPath,
			vector<ModelBlock>& modelBlocks);
	};
}