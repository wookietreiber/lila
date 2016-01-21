AM_CXXFLAGS = -Wall -pedantic -std=c++14 -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS

bin_PROGRAMS = lilac

lilac_SOURCES = lilac.cpp
lilac_LDADD = liblilac.a -lLLVM

noinst_LIBRARIES = liblilac.a

liblilac_a_SOURCES = \
	ast.hpp \
	codegen.hpp \
	codegen.cpp \
	lexer.hpp \
	lexer.cpp \
	parser.hpp \
	parser.cpp \
	token.hpp
