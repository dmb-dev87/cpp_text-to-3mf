#include <iostream>
#include <string>
#include <stdio.h>
#include <algorithm>
#include <fstream>
#include "NMR_DLLInterfaces.h"
using namespace NMR;
using namespace std;

MODELMESHVERTEX fnCreateVertex(float x, float y, float z)
{
	MODELMESHVERTEX result;
	result.m_fPosition[0] = x;
	result.m_fPosition[1] = y;
	result.m_fPosition[2] = z;
	return result;
}
MODELMESHTRIANGLE fnCreateTriangle(int v0, int v1, int v2)
{
	MODELMESHTRIANGLE result;
	result.m_nIndices[0] = v0;
	result.m_nIndices[1] = v1;
	result.m_nIndices[2] = v2;
	return result;
}
MODELMESHCOLOR_SRGB fnCreateColor(unsigned char red, unsigned char green, unsigned char blue)
{
	MODELMESHCOLOR_SRGB result;
	result.m_Red = red;
	result.m_Green = green;
	result.m_Blue = blue;
	result.m_Alpha = 255;
	return result;


}

wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

int main(int argc, char* argv[])
{
	char* fileName = new char[1000];
	char* outputName = new char[1000];
	if (argc == 1) {
		cout << "Input file Name ";
		cin >> fileName;
	}
	else {
		fileName = argv[1];
	}
	int ci;
	for (ci = 0; ci < strlen(fileName) && fileName[ci] != '.'; ci++)
		outputName[ci] = fileName[ci];
	outputName[ci] = '.';	outputName[ci+1] = '3';	outputName[ci+2] = 'm';	outputName[ci+3] = 'f';	outputName[ci+4] = '\0';

	cout << "Generating 3MF Model ..." << endl;
	// General Variables
	HRESULT hResult;
	DWORD nInterfaceVersionMajor, nInterfaceVersionMinor, nInterfaceVersionMicro;
	DWORD nErrorMessage;
	LPCSTR pszErrorMessage;

	// Objects
	PLib3MFModel* pModel;
	PLib3MFModelWriter* p3MFWriter;
	PLib3MFModelBuildItem* pBuildItem;
	PLib3MFPropertyHandler* pPropertyHandler;
	PLib3MFDefaultPropertyHandler* pDefaultPropertyHandler;

	// Check 3MF Library Version
	hResult = lib3mf_getinterfaceversion(&nInterfaceVersionMajor, &nInterfaceVersionMinor, &nInterfaceVersionMicro);

	// Create Model Instance
	hResult = lib3mf_createmodel(&pModel);
	//----------------------------------------------------		Read file	--------------------------------------------------
	ifstream inFile;
	inFile.open(fileName);
	if (!inFile) {
		cout << "Unable to open file";
		return 0;
	}
	

	string strModelCount;
	string strColor;
	string strVertex;
	string strTriangle;
	string strTemp;
	int modelCount;

	inFile >> strTemp >> modelCount;
	for (int modelId = 0; modelId < modelCount; modelId ++) {
		int colorR, colorG, colorB;
		int verticiesCount, trianglesCount;
		inFile >> strTemp >> colorR >> colorG >> colorB;
		inFile >> strTemp >> verticiesCount;
		inFile >> strTemp >> trianglesCount;

		//##############################################################################	Mesh Object	Start
		// Create Mesh Object
		PLib3MFModelMeshObject* pMeshObject;
		hResult = lib3mf_model_addmeshobject(pModel, &pMeshObject);
		if (hResult != LIB3MF_OK) {
			std::cout << "could not add mesh object: " << std::hex << hResult << std::endl;
			lib3mf_getlasterror(pModel, &nErrorMessage, &pszErrorMessage);
			std::cout << "error #" << std::hex << nErrorMessage << ": " << pszErrorMessage << std::endl;
			lib3mf_release(pModel);
			return -1;
		}


		hResult = lib3mf_object_setnameutf8(pMeshObject, "Colored Box");
		if (hResult != LIB3MF_OK) {
			std::cout << "could not set object name: " << std::hex << hResult << std::endl;
			lib3mf_getlasterror(pMeshObject, &nErrorMessage, &pszErrorMessage);
			std::cout << "error #" << std::hex << nErrorMessage << ": " << pszErrorMessage << std::endl;
			lib3mf_release(pMeshObject);
			lib3mf_release(pModel);
			return -1;
		}

		// Create mesh structure of a cube
		MODELMESHVERTEX* pVertices = new MODELMESHVERTEX[verticiesCount];
		MODELMESHTRIANGLE *pTriangles = new MODELMESHTRIANGLE[trianglesCount];
		for (int i = 0; i < verticiesCount; i++) {
			float x, y, z;
			inFile >> x >> y >> z;
			pVertices[i] = fnCreateVertex(x, y, z);
		}
		for (int i = 0; i < trianglesCount; i++) {
			int pt1, pt2, pt3;
			inFile >> pt1 >> pt2 >> pt3;
			pTriangles[i] = fnCreateTriangle(pt1, pt2, pt3);
		}

		hResult = lib3mf_meshobject_setgeometry(pMeshObject, pVertices, verticiesCount, pTriangles, trianglesCount);
		if (hResult != LIB3MF_OK) {
			std::cout << "could not set mesh geometry: " << std::hex << hResult << std::endl;
			lib3mf_getlasterror(pMeshObject, &nErrorMessage, &pszErrorMessage);
			std::cout << "error #" << std::hex << nErrorMessage << ": " << pszErrorMessage << std::endl;
			lib3mf_release(pMeshObject);
			lib3mf_release(pModel);
			return -1;
		}

		MODELMESHCOLOR_SRGB sColorRed = fnCreateColor(colorR, colorG, colorB);

		hResult = lib3mf_object_createdefaultpropertyhandler(pMeshObject, &pDefaultPropertyHandler);
		if (hResult != LIB3MF_OK) {
			std::cout << "could not create default property handler: " << std::hex << hResult << std::endl;
			lib3mf_getlasterror(pMeshObject, &nErrorMessage, &pszErrorMessage);
			std::cout << "error #" << std::hex << nErrorMessage << ": " << pszErrorMessage << std::endl;
			lib3mf_release(pMeshObject);
			lib3mf_release(pModel);
			return -1;
		}

		lib3mf_defaultpropertyhandler_setcolor(pDefaultPropertyHandler, &sColorRed);
		lib3mf_release(pDefaultPropertyHandler);

		// Add Build Item for Mesh
		hResult = lib3mf_model_addbuilditem(pModel, pMeshObject, NULL, &pBuildItem);
		if (hResult != LIB3MF_OK) {
			std::cout << "could not create build item: " << std::hex << hResult << std::endl;
			lib3mf_getlasterror(pModel, &nErrorMessage, &pszErrorMessage);
			std::cout << "error #" << std::hex << nErrorMessage << ": " << pszErrorMessage << std::endl;
			lib3mf_release(pMeshObject);
			lib3mf_release(pModel);
			return -1;
		}

		// Output cube as STL and 3MF
		lib3mf_release(pMeshObject);
		lib3mf_release(pBuildItem);
		//############################################################################		Mesh Object end
	}
	inFile.close();


	// Create Model Writer
	hResult = lib3mf_model_querywriter(pModel, "3mf", &p3MFWriter);

	// Export Model into File
	cout << "writing " << outputName << " ... " << endl;
	hResult = lib3mf_writer_writetofile(p3MFWriter, convertCharArrayToLPCWSTR(outputName));

	// Release Model Writer
	lib3mf_release(p3MFWriter);

	// Release Model
	lib3mf_release(pModel);

	cout << "done" << endl;
	return 0;
}

