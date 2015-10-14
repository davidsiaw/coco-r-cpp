#include <ruby.h>
#include <iostream>
#include <stdio.h>

#include "Scanner.h"
#include "Parser.h"
#include "Tab.h"

using namespace Coco;

// --------------
// UTILITIES
// --------------

std::wstring RSTRING_PTR_WIDE(VALUE rubyString)
{
	std::wstring wc( RSTRING_LEN(rubyString), L'#' );
	mbstowcs( &wc[0], RSTRING_PTR(rubyString), RSTRING_LEN(rubyString) );
	return wc;
}

VALUE rb_str_new_wide(std::wstring str)
{
	std::string s( str.begin(), str.end() );
	return rb_str_new(s.c_str(), s.length());
}

// --------------
// METHODS
// --------------

VALUE method_test(VALUE rubySelf, VALUE rubyString)
{
	std::wstring s(RSTRING_PTR_WIDE(rubyString));
	s = L"test-" + s;
	
	return rb_str_new_wide(s);
}

VALUE method_compile(VALUE rubySelf, VALUE rubySrcName, VALUE rubyFramedir, VALUE rubyNsName, VALUE rubyOutDir)
{
	std::wstring srcNameW = RSTRING_PTR_WIDE(rubySrcName);
	std::wstring nsNameW = RSTRING_PTR_WIDE(rubyNsName);
	std::wstring frameDirW = RSTRING_PTR_WIDE(rubyFramedir);
	std::wstring outDirW = RSTRING_PTR_WIDE(rubyOutDir);

	const wchar_t *srcName = srcNameW.c_str(), 
			*nsName = nsNameW.c_str(), 
			*frameDir = frameDirW.c_str(), 
			*outDir = outDirW.c_str(), 
			*ddtString = NULL, 
			*traceFileName = NULL;

	char *chTrFileName = NULL;
	bool emitLines = false;

	// basically copy pasta from Coco.cpp

	int pos = coco_string_lastindexof(srcName, '/');
	if (pos < 0) pos = coco_string_lastindexof(srcName, '\\');
	wchar_t* file = coco_string_create(srcName);
	wchar_t* srcDir = coco_string_create(srcName, 0, pos+1);

	Coco::Scanner *scanner = new Coco::Scanner(file);
	Coco::Parser  *parser  = new Coco::Parser(scanner);

	traceFileName = coco_string_create_append(srcDir, L"trace.txt");
	chTrFileName = coco_string_create_char(traceFileName);

	if ((parser->trace = fopen(chTrFileName, "w")) == NULL) {
		wprintf(L"-- could not open %hs\n", chTrFileName);
		exit(1);
	}

	parser->tab  = new Coco::Tab(parser);
	parser->dfa  = new Coco::DFA(parser);
	parser->pgen = new Coco::ParserGen(parser);

	parser->tab->srcName  = coco_string_create(srcName);
	parser->tab->srcDir   = coco_string_create(srcDir);
	parser->tab->nsName   = nsName ? coco_string_create(nsName) : NULL;
	parser->tab->frameDir = coco_string_create(frameDir);
	parser->tab->outDir   = coco_string_create(outDir != NULL ? outDir : srcDir);
	parser->tab->emitLines = emitLines;

	if (ddtString != NULL) parser->tab->SetDDT(ddtString);

	parser->Parse();

	fclose(parser->trace);

	// obtain the FileSize
	parser->trace = fopen(chTrFileName, "r");
	fseek(parser->trace, 0, SEEK_END);
	long fileSize = ftell(parser->trace);
	fclose(parser->trace);
	if (fileSize == 0) {
		remove(chTrFileName);
	} else {
		wprintf(L"trace output is in %hs\n", chTrFileName);
	}

	wprintf(L"%d errors detected\n", parser->errors->count);
	if (parser->errors->count != 0) {
		exit(1);
	}

	delete parser->pgen;
	delete parser->dfa;
	delete parser->tab;
	delete parser;
	delete scanner;
	coco_string_delete(file);
	coco_string_delete(srcDir);

	return Qnil;
}

extern "C" void Init_cocor()
{
	VALUE cocor = rb_define_module("Cocor");
	rb_define_method(cocor, "cocor_test", (VALUE(*)(ANYARGS))method_test,1);
	rb_define_method(cocor, "cocor_compile", (VALUE(*)(ANYARGS))method_compile,4);
}
