/*****************************************************************//**
 * \file   HttpRouter.h
 * \brief  routes web services
 * 
 * \author binlee0903
 * \date   February 2023
 *********************************************************************/

#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_map>

#include "HttpObject.h"
#include "HttpFileContainer.h"


class HttpRouter final
{
public:
	HttpRouter();
	~HttpRouter();

	void CreateHeader(HttpObject* httpObject, std::vector<int8_t>* header) const;
	void WriteServiceFileToVector(HttpObject* httpObject, std::vector<int8_t>* responseBody) const;

private:


	bool isHasService(std::string& path) const;


private:
	std::hash<std::string> mStringHash;
	HttpFileContainer* mTextFileContainer;
};

