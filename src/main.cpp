//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <sstream>

#include "KalaCLI/include/core.hpp"
#include "KalaCLI/include/command.hpp"

#include "parse.hpp"

using KalaCLI::Core;
using KalaCLI::Command;
using KalaCLI::CommandManager;

using KalaModel::Parse;

using std::ostringstream;

static void AddExternalCommands()
{
	ostringstream msgParse{};
	
	msgParse << "Compiles models to kmd for runtime use with the help of Assimp.\n"
		<< "    Second parameter must be downscale size\n"
		<< "    Third parameter must be origin model path (.gltf, .obj or .fbx)\n"
		<< "    Fourth parameter must be target path (.kmd)";
	
	ostringstream msgVerboseParse{};
	
	msgVerboseParse << "Compiles models to kmd for runtime use with the help of Assimp with additional verbose logging.\n"
		<< "    Second parameter must be downscale size\n"
		<< "    Third parameter must be origin model path (.gltf, .obj or .fbx)\n"
		<< "    Fourth parameter must be target path (.kmd)";
	
	Command cmd_parse
	{
		.primary = { "parse", "p" },
		.description = msgParse.str(),
		.paramCount = 4,
		.targetFunction = Parse::Command_Parse
	};
	Command cmd_verboseparse
	{
		.primary = { "vp" },
		.description = msgVerboseParse.str(),
		.paramCount = 4,
		.targetFunction = Parse::Command_VerboseParse
	};

	CommandManager::AddCommand(cmd_parse);
	CommandManager::AddCommand(cmd_verboseparse);
}

int main(int argc, char* argv[])
{
	Core::Run(argc, argv, AddExternalCommands);

	return 0;
}