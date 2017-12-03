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
#pragma once

#include <stdint.h>

namespace Sandworm
{
	struct Preprocessor
	{
		struct Result
		{
			virtual ~Result() {}
			virtual const char* GetResult() const = 0;
			virtual uint32_t GetWarningsCount() const = 0;
			virtual const char* GetWarningString(uint32_t warningIndex) const = 0;
			virtual uint32_t GetErrorsCount() const = 0;
			virtual const char* GetErrorString(uint32_t errorIndex) const = 0;

			//static Result* Create();
			static void Destroy(Result* p);
		};

		virtual ~Preprocessor() {}

		virtual void AddFile(const char* fileName, const char* fileContent) = 0;
		virtual Result* DoPreprocess(const char* fileName, const char** pDefines, uint32_t definesCount) = 0;

		static Preprocessor* Create();
		static void Destroy(Preprocessor* p);
	};

	struct PreprocessorDeleter {
		void operator()(Preprocessor* p) { Preprocessor::Destroy(p); }
	};

	struct ResultDeleter {
		void operator()(Preprocessor::Result* p) { Preprocessor::Result::Destroy(p); }
	};

}

