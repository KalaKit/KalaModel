//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <vector>
#include <string>

namespace KalaModel
{
	using std::vector;
	using std::string;
	
	class Parse
	{
	public:
		//Compiles models to kmf for runtime use with the help of Assimp.
		static void Command_Parse(const vector<string>& params);
		
		//Compiles models to kmf for runtime use
		//with the help of Assimp with additional verbose logging.
		static void Command_VerboseParse(const vector<string>& params);
	};
}