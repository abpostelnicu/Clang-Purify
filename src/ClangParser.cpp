#include "ClangParser.h"

void ClangParser::prepareCompilerforFile(const char* szSourceCodeFilePath) {
  // To reuse the source manager we have to create these tables.
  m_CompilerInstance.getSourceManager().clearIDTables();

  // supply the file to the source manager and set it as main-file
  const clang::FileEntry * file = m_CompilerInstance.getFileManager().getFile(szSourceCodeFilePath);
  clang::FileID fileID = m_CompilerInstance.getSourceManager().createFileID(file, clang::SourceLocation(), clang::SrcMgr::CharacteristicKind::C_User);
  m_CompilerInstance.getSourceManager().setMainFileID(fileID);

  // CodeGenAction needs this
  clang::FrontendOptions& feOptions = m_CompilerInstance.getFrontendOpts();
  feOptions.Inputs.clear();
  feOptions.Inputs.push_back(clang::FrontendInputFile(szSourceCodeFilePath,
                                                      clang::FrontendOptions::getInputKindForExtension(clang::StringRef(szSourceCodeFilePath).rsplit('.').second), false));
}

ClangParser::ClangParser() {
  // Diagnostics is needed - set it up
  m_CompilerInstance.createDiagnostics();

  // set few options
  clang::LangOptions& langOptions = m_CompilerInstance.getLangOpts();
  langOptions.CPlusPlus = 1;
  langOptions.CPlusPlus11 = 1;
  langOptions.Bool = 1;
  langOptions.RTTI = 0;

  // Need to set the source manager before AST
  m_CompilerInstance.createFileManager();
  m_CompilerInstance.createSourceManager(m_CompilerInstance.getFileManager());

  // Need to set the target before AST. Adjust the default target options and create a target
  m_CompilerInstance.getTargetOpts().Triple = llvm::sys::getProcessTriple();  
  std::shared_ptr<clang::TargetOptions> ptr(&m_CompilerInstance.getTargetOpts());
  m_CompilerInstance.setTarget(clang::TargetInfo::CreateTargetInfo(m_CompilerInstance.getDiagnostics(), ptr));

  // Create pre-processor and AST Context
  m_CompilerInstance.createPreprocessor(clang::TranslationUnitKind::TU_Module);
  m_CompilerInstance.createASTContext();
  if (m_CompilerInstance.hasPreprocessor()) {
    clang::Preprocessor & preprocessor = m_CompilerInstance.getPreprocessor();
    preprocessor.getBuiltinInfo().initializeBuiltins(preprocessor.getIdentifierTable(), preprocessor.getLangOpts());
  }
}

bool ClangParser::parseAST(const char* szSourceCodeFilePath, clang::ast_matchers::MatchFinder finder) {
  // create the compiler instance setup for this file as main file
  prepareCompilerforFile(szSourceCodeFilePath);

  std::unique_ptr<clang::ASTConsumer> pAstConsumer(finder.newASTConsumer());

  clang::DiagnosticConsumer& diagConsumer = m_CompilerInstance.getDiagnosticClient();
  diagConsumer.BeginSourceFile(m_CompilerInstance.getLangOpts(), &m_CompilerInstance.getPreprocessor());
  clang::ParseAST(m_CompilerInstance.getPreprocessor(), pAstConsumer.get(), m_CompilerInstance.getASTContext());
  diagConsumer.EndSourceFile();

  return diagConsumer.getNumErrors() != 0;
}

ClangParser::~ClangParser() {
  
}
