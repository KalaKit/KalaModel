//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <vector>
#include <array>
#include <string>
#include <filesystem>

#include "Assimp/include/Importer.hpp"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"

#include "KalaHeaders/log_utils.hpp"
#include "KalaHeaders/math_utils.hpp"
#include "KalaHeaders/string_utils.hpp"
#include "KalaHeaders/import_kmd.hpp"

#include "KalaCLI/include/core.hpp"

#include "parse.hpp"
#include "export.hpp"

using Assimp::Importer;

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;
using KalaHeaders::KalaMath::vec2;
using KalaHeaders::KalaMath::vec3;
using KalaHeaders::KalaMath::vec4;
using KalaHeaders::KalaMath::normalize;
using KalaHeaders::KalaMath::dot;
using KalaHeaders::KalaMath::length;
using KalaHeaders::KalaMath::cross;
using KalaHeaders::KalaModelData::ModelBlock;
using KalaHeaders::KalaModelData::Vertex;

using KalaCLI::Core;

using KalaModel::Export;

using std::vector;
using std::array;
using std::string;
using std::string_view;
using std::to_string;
using std::filesystem::current_path;
using std::filesystem::path;
using std::filesystem::weakly_canonical;
using std::filesystem::is_regular_file;
using std::filesystem::exists;
using std::filesystem::status;
using std::filesystem::perms;

//Adjusts final imported model size by this scale
constexpr f32 SCALE_MULTIPLIER = 0.01f;

constexpr array<string_view, 3> allowedExtensions
{
	".fbx",
	".obj",
	".gltf"
};

struct Mesh
{
	aiMesh* mesh{};
	string meshName{};
};
struct Node
{
	aiNode* node{};
	string nodeName{};
	string nodePath{};
	vector<Mesh> meshes{};
};

static void ParseAny(
	const vector<string>& params,
	bool isVerbose);
	
static void GetAllNodes(
	const aiScene* scene, 
	aiNode* node, 
	vector<Node>& out);
	
static void PrintError(const string& message)
{
	Log::Print(
		message,
		"PARSE",
		LogType::LOG_ERROR,
		2);
}

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
	u32 scaleFactorWide = stoul(params[1]);
	u8 scaleFactor = static_cast<u8>(
		clamp(scaleFactorWide,
		0u,
		8u));
		
	path correctOrigin = weakly_canonical(path(Core::currentDir) / params[2]);
	path correctTarget = weakly_canonical(path(Core::currentDir) / params[3]);
	
	//
	// VERIFY ORIGIN
	//
	
	if (!exists(correctOrigin))
	{
		PrintError("Failed to load model because input path '" + correctOrigin.string() + "' does not exist!");
		
		return;
	}
	
	if (!is_regular_file(correctOrigin)
		|| !correctOrigin.has_extension())
	{
		PrintError("Failed to load model because input path '" + correctOrigin.string() + "' is not a regular file!");
		
		return;
	}
	
	if (find(
		allowedExtensions.begin(),
		allowedExtensions.end(),
		correctOrigin.extension().string())
		== allowedExtensions.end())
	{
		PrintError("Failed to load model because input path '" + correctOrigin.string() + "' extension '" + correctOrigin.extension().string() + "' is not allowed!");
		
		return;
	}
	
	auto fileStatusOrigin = status(correctOrigin);
	auto filePermsOrigin = fileStatusOrigin.permissions();
	
	bool canReadOrigin = (filePermsOrigin & (
		perms::owner_read
		| perms::group_read
		| perms::others_read))
		!= perms::none;
		
	if (!canReadOrigin)
	{
		PrintError("Failed to load model because you have insufficient read permissions for input path '" + correctOrigin.string() + "'!");
		
		return;
	}
	
	//
	// VERIFY TARGET
	//
	
	if (exists(correctTarget))
	{
		PrintError("Failed to load model because output path '" + correctTarget.string() + "' already exists!");
		
		return;
	}
	
	if (!correctTarget.has_extension()
		|| correctTarget.extension() != ".kmd")
	{
		PrintError("Failed to load model because output path '" + correctTarget.string() + "' extension '" + correctTarget.extension().string() + "' is not allowed!");
		
		return;
	}
	
	auto fileStatusTarget = status(correctTarget.parent_path());
	auto filePermsTarget = fileStatusTarget.permissions();
	
	bool canWriteTarget = (filePermsTarget & (
		perms::owner_write
		| perms::group_write
		| perms::others_write))
		!= perms::none;
		
	if (!canWriteTarget)
	{
		PrintError("Failed to load model because you have insufficient write permissions for output parent path '" + correctTarget.string() + "'!");
		
		return;
	}
	
	//
	// INITIALIZE ASSIMP
	//
	
	Importer importer{};
	const aiScene* scene{};
	
	scene = importer.ReadFile(
		correctOrigin.string(),
		aiProcess_Triangulate
		| aiProcess_GenSmoothNormals
		| aiProcess_FlipUVs
		| aiProcess_JoinIdenticalVertices
	);
	
	if (!scene
		|| !scene->mRootNode
		|| scene->mNumMeshes == 0)
	{
		PrintError("Failed to load model because input path '" + correctTarget.string() + "' points to a broken or empty model file!");
		
		return;
	}
	
	//
	// GET ALL ASSIMP NODES
	//
	
	vector<Node> nodes{};
	
	GetAllNodes(scene, scene->mRootNode, nodes);
	
	if (nodes.empty())
	{
		PrintError("Failed to load model because input path '" + correctTarget.string() + "' has no nodes!");
		
		return;
	}
	
	//
	// GET TRANSFORM, VERTICES AND INDICES
	//
	
	vector<ModelBlock> models{};
	
	for (const auto& n : nodes)
	{
		aiNode* node = n.node;
		
		//get full transform per node
		
		aiMatrix4x4 fullTransform = node->mTransformation;
		aiNode* parent = node->mParent;
		
		while (parent)
		{
			fullTransform = parent->mTransformation * fullTransform;
			parent = parent->mParent;
		}
		
		aiVector3D scaling{};
		aiQuaternion rotation{};
		aiVector3D position{};
		
		fullTransform.Decompose(scaling, rotation, position);
		
		//get vertices and indices
		
		for (const auto& m : n.meshes)
		{
			aiMesh* mesh = m.mesh;
			
			ModelBlock b{};
			
			b.position[0] = position.x;
			b.position[1] = position.y;
			b.position[2] = position.z;
			
			b.rotation[0] = rotation.w;
			b.rotation[1] = rotation.x;
			b.rotation[2] = rotation.y;
			b.rotation[3] = rotation.z;
			
			b.size[0] = scaling.x;
			b.size[1] = scaling.y;
			b.size[2] = scaling.z;
		
			//vertices
			b.vertices.reserve(mesh->mNumVertices);
			for (u32 i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex v{};
				
				v.position[0] = mesh->mVertices[i].x * SCALE_MULTIPLIER;
				v.position[1] = mesh->mVertices[i].y * SCALE_MULTIPLIER;
				v.position[2] = mesh->mVertices[i].z * SCALE_MULTIPLIER;
				
				if (mesh->HasNormals())
				{
					vec3 norm =
					{
						mesh->mNormals[i].x,
						mesh->mNormals[i].y,
						mesh->mNormals[i].z
					};
					norm = normalize(norm);
					
					v.normal[0] = norm.x;
					v.normal[1] = norm.y;
					v.normal[2] = norm.z;
				}
				
				if (mesh->HasTextureCoords(0))
				{
					v.texCoord[0] = mesh->mTextureCoords[0][i].x;
					v.texCoord[1] = mesh->mTextureCoords[0][i].y;
				}
				
				b.vertices.push_back(v);
			}
			
			//indices
			b.indices.reserve(mesh->mNumFaces * 3);
			for (u32 f = 0; f < mesh->mNumFaces; f++)
			{
				aiFace face = mesh->mFaces[f];
				for (u32 j = 0; j < face.mNumIndices; j++)
				{
					b.indices.push_back(face.mIndices[j]);
				}
			}
			
			models.push_back(move(b));
		}
	}
	
	//
	// GENERATE TANGENTS
	//
	
	for (auto& b : models)
	{
		vector<vec3> tan1(b.vertices.size(), vec3(0));
		vector<vec3> tan2(b.vertices.size(), vec3(0));
		
		//accumulate tangents/bitangents
		for (size_t i = 0; i < b.indices.size(); i += 3)
		{
			u32 i1 = b.indices[i + 0];
			u32 i2 = b.indices[i + 1];
			u32 i3 = b.indices[i + 2];
			
			Vertex& v1 = b.vertices[i1];
			Vertex& v2 = b.vertices[i2];
			Vertex& v3 = b.vertices[i3];
			
			vec3 p1 = v1.position;
			vec3 p2 = v2.position;
			vec3 p3 = v3.position;
			
			vec2 w1 = v1.texCoord;
			vec2 w2 = v2.texCoord;
			vec2 w3 = v3.texCoord;
			
			f32 x1 = p2.x - p1.x;
			f32 x2 = p3.x - p1.x;
			f32 y1 = p2.y - p1.y;
			f32 y2 = p3.y - p1.y;
			f32 z1 = p2.z - p1.z;
			f32 z2 = p3.z - p1.z;

			f32 s1 = w2.x - w1.x;
			f32 s2 = w3.x - w1.x;
			f32 t1 = w2.y - w1.y;
			f32 t2 = w3.y - w1.y;
			
			f32 r = (s1 * t2 - s2 * t1);
			if (fabs(r) < 1e-6f) r = 1.0f;
			else r = 1.0f / r;
			
			vec3 sdir(
				(t2 * x1 - t1 * x2) * r,
				(t2 * y1 - t1 * y2) * r,
				(t2 * z1 - t1 * z2) * r);

			vec3 tdir(
				(s1 * x2 - s2 * x1) * r,
				(s1 * y2 - s2 * y1) * r,
				(s1 * z2 - s2 * z1) * r);

			tan1[i1] += sdir;
			tan1[i2] += sdir;
			tan1[i3] += sdir;

			tan2[i1] += tdir;
			tan2[i2] += tdir;
			tan2[i3] += tdir;
		}

		//orthogonalize and store handedness
		for (size_t i = 0; i < b.vertices.size(); i++)
		{
			vec3 n = normalize(vec3(b.vertices[i].normal));
			vec3 t = tan1[i];
			
			//gram-schmidt orthogonalize
			vec3 tangent = t - n * dot(n, t);
			
			if (length(tangent) < 1e-6) tangent = vec3(1, 0, 0);
			else tangent = normalize(tangent);
			
			//calculate handedness
			float w = (dot(cross(n, tangent), tan2[i]) < 0.0f)
				? 1.0f
				: 0.0f;
				
			b.vertices[i].tangent[0] = tangent.x;
			b.vertices[i].tangent[1] = tangent.y;
			b.vertices[i].tangent[2] = tangent.z;
			b.vertices[i].tangent[3] = w;
		}
	}
	
	//
	// FINALIZE AND EXIT
	//
	
	Export::ExportKMF(
		correctTarget,
		scaleFactor,
		models);
}

void GetAllNodes(
	const aiScene* scene,
	aiNode* node,
	vector<Node>& out)
{
	//store all found meshes and their hierarchy paths
	if (node->mNumMeshes > 0)
	{
		Node n{};
	
		string nodePath = node->mName.C_Str();
		aiNode* parent = node->mParent;
	
		while (parent)
		{
			nodePath = string(parent->mName.C_Str()) + "/" + nodePath;
			parent = parent->mParent;
		}
			
		string nodeName = node->mName.C_Str();
		string suffix = "/" + nodeName;
				
		//strip trailing node name from path
		if (nodePath == nodeName) nodePath.clear();
		else if (nodePath.size() > suffix.size()
			&& nodePath.rfind(suffix) == nodePath.size() - suffix.size())
		{
			nodePath.erase(nodePath.size() - suffix.size(), suffix.size());	
		}
			
		for (u32 i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			
			string meshName = mesh->mName.length > 0
				? mesh->mName.C_Str()
				: nodeName + "_mesh" + to_string(i);
				
			n.meshes.push_back(
			{
				.mesh = mesh,
				.meshName = meshName
			});
		}
				
		n.node = node;
		n.nodeName = nodeName;
		n.nodePath = nodePath;
			
		out.push_back(n);
	}
			
	//recurse into children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		GetAllNodes(scene, node->mChildren[i], out);
	}
};