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
#include <stdarg.h>

#pragma warning( push )
#pragma warning( disable : 4141 ) // warning C4141: 'inline': used more than once
#pragma warning( disable : 4146 ) // warning C4146: unary minus operator applied to unsigned type, result still unsigned
#pragma warning( disable : 4244 ) // warning C4244: 'argument': conversion from 'uint64_t' to '::size_t', possible loss of data
#pragma warning( disable : 4996 ) // warning C4996: 'std::copy::_Unchecked_iterators::_Deprecate': Call to 'std::copy' with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct. To disable this warning, use -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++ 'Checked Iterators'

#include "clang/Frontend/PreprocessorOutputOptions.h"
#include "clang/Frontend/Utils.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Pragma.h"
#include "llvm/ADT/SmallString.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"

#pragma warning( pop )



#pragma comment( lib, "clangFrontend.lib" )
#pragma comment( lib, "clangBasic.lib" )
#pragma comment( lib, "clangLex.lib" )
#pragma comment( lib, "clangSema.lib" )
#pragma comment( lib, "clangAST.lib" )
#pragma comment( lib, "clangSerialization.lib" )
#pragma comment( lib, "clangParse.lib" )
#pragma comment( lib, "clangDriver.lib" )
#pragma comment( lib, "clangEdit.lib" )
#pragma comment( lib, "clangAnalysis.lib" )

#pragma comment( lib, "LLVMBinaryFormat.lib" )
#pragma comment( lib, "LLVMCore.lib" )
#pragma comment( lib, "LLVMSupport.lib" )
#pragma comment( lib, "LLVMMC.lib" )
#pragma comment( lib, "LLVMBitReader.lib" )
#pragma comment( lib, "LLVMMCParser.lib" )
#pragma comment( lib, "LLVMOption.lib" )
#pragma comment( lib, "LLVMProfileData.lib" )


//#pragma comment( lib, "Mincore.lib" )


#define SOURCE_FILES_ROOT "sandworm"

namespace Sandworm
{


	class ResultImpl : public Preprocessor::Result
	{
		std::string results;
		std::vector<std::string> warnings;
		std::vector<std::string> errors;

	public:

		ResultImpl()
		{
		}

		virtual ~ResultImpl()
		{
		}

		virtual const char* GetResult() const override
		{
			return results.c_str();
		}

		virtual uint32_t GetWarningsCount() const override
		{
			return (uint32_t)warnings.size();
		}

		virtual const char* GetWarningString(uint32_t warningIndex) const override
		{
			return warnings[warningIndex].c_str();
		}

		virtual uint32_t GetErrorsCount() const override
		{
			return (uint32_t)errors.size();
		}

		virtual const char* GetErrorString(uint32_t errorIndex) const override
		{
			return errors[errorIndex].c_str();
		}

		std::string& GetMutableResultString()
		{
			return results;
		}

		void AddError(const char* desc)
		{
			errors.push_back(desc);
		}

		void AddWarning(const char* desc)
		{
			warnings.push_back(desc);
		}


		static ResultImpl* Create()
		{
			return new ResultImpl();
		}
	};


	void Preprocessor::Result::Destroy(Preprocessor::Result* p)
	{
		delete p;
	}


	class PreprocessorImpl : public Preprocessor
	{
		class CLangDiagnostic : public clang::DiagnosticConsumer
		{
			PreprocessorImpl* preprocessor;
			int errorsCount;
			int warningsCount;

		public:

			CLangDiagnostic(PreprocessorImpl* _preprocessor)
				: preprocessor(_preprocessor)
			{
				ResetErrorsCounter();
			}

			virtual void BeginSourceFile(const clang::LangOptions &LangOpts, const clang::Preprocessor *PP = 0) override
			{
			}

			virtual void HandleDiagnostic(clang::DiagnosticsEngine::Level Level, const clang::Diagnostic &Info) override
			{
				if (Level == clang::DiagnosticsEngine::Warning)
				{
					warningsCount++;
				}
				else
				{
					if (Level >= clang::DiagnosticsEngine::Error)
					{
						errorsCount++;
					}
				}

				const clang::SourceManager& sm = Info.getSourceManager();

				clang::SmallString<4096> messageStr;
				Info.FormatDiagnostic(messageStr);
				const clang::SourceLocation& sourceLocation = Info.getLocation();
				messageStr.append(" (");
				messageStr.append(sourceLocation.printToString(sm));
				messageStr.append(")");
				preprocessor->ReportError(Level, messageStr.c_str());
			}

			void ResetErrorsCounter()
			{
				errorsCount = 0;
				warningsCount = 0;
			}

			bool HaveErrors() const
			{
				return errorsCount > 0;
			}

			bool HaveWarinngs() const
			{
				return warningsCount > 0;
			}
		};

		class CLangDependencyGraphCallback : public clang::PPCallbacks
		{
			std::set<std::string>* includes;

		public:

			CLangDependencyGraphCallback()
				: includes(nullptr)
			{
			}

			~CLangDependencyGraphCallback()
			{
			}

			void SetTarget(std::set<std::string> & _includes)
			{
				includes = &_includes;
			}

			virtual void InclusionDirective(clang::SourceLocation HashLoc,
				const clang::Token &IncludeTok,
				llvm::StringRef FileName,
				bool IsAngled,
				clang::CharSourceRange FilenameRange,
				const clang::FileEntry *File,
				llvm::StringRef SearchPath,
				llvm::StringRef RelativePath,
				const clang::Module *Imported)
			{
				if (includes)
				{
					clang::SmallString<4096> dependencyFileName(FileName);
					const char* includedFile = dependencyFileName.c_str();
					includes->insert(includedFile);
				}
			}

			virtual void EndOfMainFile()
			{
			}
		};



		void Create()
		{
			diagnosticClient = new CLangDiagnostic(this);

			diagnosticOptions = llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions>(new clang::DiagnosticOptions());
			diagnosticOptions->setFormat(clang::DiagnosticOptions::MSVC);

			diagIDs = llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs>(new clang::DiagnosticIDs());
			diagnosticEngine = llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine>(new clang::DiagnosticsEngine(diagIDs, diagnosticOptions.get(), diagnosticClient));

			clang::FileSystemOptions fsOptions;
			fileManager = llvm::IntrusiveRefCntPtr<clang::FileManager>(new clang::FileManager(fsOptions));
			sourceManager = llvm::IntrusiveRefCntPtr<clang::SourceManager>(new clang::SourceManager(*diagnosticEngine.get(), *fileManager.get()));

			std::shared_ptr<clang::TargetOptions> options(new clang::TargetOptions());
			options->Triple = "x86_64";
			target = llvm::IntrusiveRefCntPtr<clang::TargetInfo>(clang::TargetInfo::CreateTargetInfo(*diagnosticEngine.get(), options));

			compiler.setDiagnostics(diagnosticEngine.get());
			compiler.setFileManager(fileManager.get());
			compiler.setSourceManager(sourceManager.get());
			compiler.setTarget(target.get());

			clang::LangOptions& languageOpts = compiler.getLangOpts();
			languageOpts.CPlusPlus = true;
			languageOpts.NoBuiltin = true;
			languageOpts.AsmPreprocessor = false;
			languageOpts.Modules = false;
			languageOpts.MicrosoftExt = true;

			clang::HeaderSearchOptions& hopt = compiler.getHeaderSearchOpts();

			hopt.AddPath(SOURCE_FILES_ROOT, clang::frontend::Angled, false, true);
			hopt.AddPath(SOURCE_FILES_ROOT, clang::frontend::Quoted, false, true);
			hopt.UseStandardCXXIncludes = false;
			hopt.UseStandardSystemIncludes = false;
			hopt.UseBuiltinIncludes = false;
			hopt.Verbose = 0;
		}


		void Destroy()
		{
			diagnosticOptions.resetWithoutRelease();
			diagIDs.resetWithoutRelease();
			diagnosticEngine.resetWithoutRelease();
			fileManager.resetWithoutRelease();
			sourceManager.resetWithoutRelease();
			target.resetWithoutRelease();
		}



	public:

		PreprocessorImpl::PreprocessorImpl()
			: diagnosticClient(nullptr)
			, resultContext(nullptr)
		{
			Create();
		}

		PreprocessorImpl::~PreprocessorImpl()
		{
			Destroy();
		}


		void AdjustFileName(const char* fileName, clang::SmallString<4096>& result)
		{
			result = SOURCE_FILES_ROOT;
			result.append("/");
			result.append(fileName);
		}

		virtual void PreprocessorImpl::AddFile(const char* fileName, const char* fileContent) override
		{
			clang::FileManager & fm = *fileManager.get();
			clang::SourceManager & sm = *sourceManager.get();

			std::unique_ptr<llvm::MemoryBuffer> fileData = llvm::MemoryBuffer::getMemBufferCopy(fileContent);
			//		std::unique_ptr<llvm::MemoryBuffer> fileData3( new DebugMemoryBuffer());

			clang::SmallString<4096> fullPath;
			AdjustFileName(fileName, fullPath);

			const clang::FileEntry* virtualFile = fm.getVirtualFile(fullPath, fileData->getBufferSize(), 0);
			sm.overrideFileContents(virtualFile, std::move(fileData));
		}

		virtual Result* PreprocessorImpl::DoPreprocess(const char* fileName, const char** pDefines, uint32_t definesCount) override
		{
			clang::SmallString<4096> fullPath;
			AdjustFileName(fileName, fullPath);

			ResultImpl* result = ResultImpl::Create();
			resultContext = result;

			// Find target file
			clang::FileManager & fm = *fileManager.get();
			if (!fm.getFile(fullPath.c_str()))
			{
				clang::SmallString<4096> message;
				message.append("Can't find '");
				message.append(fullPath);
				message.append("'. All filenames are Case Sensitive!");
				ReportError(clang::DiagnosticsEngine::Error, message.c_str());
				resultContext = nullptr;
				return result;
			}

			//std::unique_ptr<llvm::MemoryBuffer> tmp = llvm::MemoryBuffer::getMemBufferCopy(1024*1024);
			/////////////////////////////////////////////////////////////////////////////////////////////////
			const clang::FileEntry* vf = fm.getVirtualFile(fullPath.c_str(), 1024 * 1024, 0);
			llvm::StringRef nm = vf->getName();
			/////////////////////////////////////////////////////////////////////////////////////////////////


			clang::PreprocessorOptions& opts = compiler.getPreprocessorOpts();
			//opts.etailedRecord = false;
			opts.TokenCache.clear();
			opts.Macros.clear();
			opts.UsePredefines = false;
			opts.ObjCXXARCStandardLibrary = clang::ARCXX_nolib;
			for (uint32_t i = 0; i < definesCount; i++)
			{
				const char* macroDef = pDefines[i];
				opts.addMacroDef(macroDef);
			}

			diagnosticClient->ResetErrorsCounter();

			compiler.getSourceManager().clearIDTables();
			clang::FrontendInputFile infile(fullPath.c_str(), clang::InputKind(clang::InputKind::C));
			compiler.InitializeSourceManager(infile);

			/////////////////////////////////////////////////////////////////////////////////////////////////
			clang::FileID mainId = sourceManager->getMainFileID();

			bool inv = false;
			llvm::MemoryBuffer* fileBuffer = sourceManager->getBuffer(mainId, &inv);
			const char* srctxt = fileBuffer->getBufferStart();
			/////////////////////////////////////////////////////////////////////////////////////////////////

			compiler.createPreprocessor(clang::TU_Complete);

			clang::PreprocessorOutputOptions preprocessorOutOptions;
			preprocessorOutOptions.ShowCPP = true;
			preprocessorOutOptions.ShowComments = false;
			preprocessorOutOptions.ShowLineMarkers = true;
			preprocessorOutOptions.UseLineDirectives = true;
			preprocessorOutOptions.ShowMacroComments = false;
			preprocessorOutOptions.ShowMacros = false;
			preprocessorOutOptions.RewriteIncludes = false;

			clang::Preprocessor& pp = compiler.getPreprocessor();

			std::unique_ptr<CLangDependencyGraphCallback> depGraph(new CLangDependencyGraphCallback());
			includes.clear();
			depGraph->SetTarget(includes);
			compiler.getPreprocessor().addPPCallbacks(std::move(depGraph));
			
			llvm::raw_string_ostream outputStream(result->GetMutableResultString());

			clang::DoPrintPreprocessedInput(pp, &outputStream, preprocessorOutOptions);

			//clang::ASTReader::ClearPreprocessorHack(&pp);
			compiler.setPreprocessor(nullptr);
			resultContext = nullptr;
			outputStream.flush();
			return result;
		}


		void ReportError(clang::DiagnosticsEngine::Level Level, const char* message)
		{
			if (!resultContext)
			{
				return;
			}

			if (Level == clang::DiagnosticIDs::Warning)
			{
				resultContext->AddWarning(message);
			}
			else if (Level >= clang::DiagnosticIDs::Error)
			{
				resultContext->AddError(message);
			}
		}


	private:

		std::set<std::string> includes;
		CLangDiagnostic* diagnosticClient;
		llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diagnosticOptions;
		llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diagIDs;
		llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diagnosticEngine;
		llvm::IntrusiveRefCntPtr<clang::FileManager> fileManager;
		llvm::IntrusiveRefCntPtr<clang::SourceManager> sourceManager;
		llvm::IntrusiveRefCntPtr<clang::TargetInfo> target;

		clang::CompilerInstance compiler;

		ResultImpl* resultContext;
	};


	Preprocessor* Preprocessor::Create()
	{
		return new PreprocessorImpl();
	}

	void Preprocessor::Destroy(Preprocessor* p)
	{
		delete p;
	}


}