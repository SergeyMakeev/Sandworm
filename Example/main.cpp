// The MIT License (MIT)
//
// 	Copyright (c) 2017 Sergey Makeev
//
// 	Permission is hereby granted, free of charge, to any person obtaining a copy
// 	of this software and associated documentation files (the "Software"), to deal
// 	in the Software without restriction, including without limitation the rights
// 	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// 	copies of the Software, and to permit persons to whom the Software is
// 	furnished to do so, subject to the following conditions:
//
//      The above copyright notice and this permission notice shall be included in
// 	all copies or substantial portions of the Software.
//
// 	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// 	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// 	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// 	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// 	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// 	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// 	THE SOFTWARE.
#include <Sandworm.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <memory>

// GetFileVersionInfoSize and GetFileVersionInfo from clang
#pragma comment( lib, "version.lib" )


std::string ReadFileToString(const char* name)
{
	std::string fpath = "Data/";
	fpath.append(name);
	std::ifstream t(fpath.c_str());
	return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

int main(/*int argc, char *argv[]*/)
{
	const char* hlslName = "shader.hlsl";
	std::string hlsl = ReadFileToString(hlslName);

	const char* headerName = "shader.inc";
	std::string header = ReadFileToString(headerName);

	std::unique_ptr<Sandworm::Preprocessor, Sandworm::PreprocessorDeleter> preprocessor( Sandworm::Preprocessor::Create() );
		
	preprocessor->AddFile(hlslName, hlsl.c_str());
	preprocessor->AddFile(headerName, header.c_str());
	
	const char* macro[] = { "UBER_SHADER=3", "ALPHA=2.0f" };

	std::unique_ptr<Sandworm::Preprocessor::Result, Sandworm::ResultDeleter> result( preprocessor->DoPreprocess(hlslName, &macro[0], 2) );

	if (!result || result->GetErrorsCount() > 0)
	{
		printf("ERROR list:\n");
		for (uint32_t i = 0; i < result->GetErrorsCount(); i++)
		{
			printf("Error : %s\n", result->GetErrorString(i));
		}
		return -1;
	}

	if (result->GetWarningsCount() > 0)
	{
		printf("Warning list:\n");
		for (uint32_t i = 0; i < result->GetWarningsCount(); i++)
		{
			printf("Warning : %s\n", result->GetWarningString(i));
		}
	}

	printf("%s\n", result->GetResult());
	return 0;
}