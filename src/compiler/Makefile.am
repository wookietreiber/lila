AM_CXXFLAGS = -Wall -pedantic -std=c++14 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -fno-exceptions

bin_PROGRAMS = lilac

lilac_LDADD = -lLLVM

lilac_SOURCES = \
	ast.hpp \
	codegen.hpp \
	codegen.cpp \
	lexer.hpp \
	lexer.cpp \
	parser.hpp \
	parser.cpp \
	token.hpp \
	lilac.cpp
