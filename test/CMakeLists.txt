# MIT License

# Copyright (c) 2017 effolkronium

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

project(random_test)

if(COVERAGE_ENABLED)
	message(STATUS "COVERAGE ENABLED")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0") # debug, no optimisation
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # enabling coverage
endif()

add_executable(${PROJECT_NAME} random_test.cpp)

if(NOT MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE 
		-Wall -Wextra # reasonable and standard
		-Werror # Make all warnings into errors.
		-Wshadow # warn the user if a variable declaration shadows one from a
				 # parent context
		-Wnon-virtual-dtor # warn the user if a class with virtual functions
						   # has a non-virtual destructor. This helps catch 
						   # hard to track down memory errors
		-pedantic
		-Wold-style-cast # warn for c-style casts
		-Wcast-align # warn for potential performance problem casts
		-Wunused # warn on anything being unused
		-Woverloaded-virtual # warn if you overload (not override) a virtual function
		-Wconversion # warn on type conversions that may lose data
		-Wsign-conversion # warn on sign conversions
	)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE 
		/EHsc # Warning fix!
		/W4 
		/WX # Treats all compilers warnings as errors
		/w14242 # 'identfier': conversion from 'type1' to 'type1', possible loss of data
		/w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
		/w14263 # 'function': member function does not override any base class virtual member function
		/w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
		/w14287 # 'operator': unsigned/negative constant mismatch
		/we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
		/w14296 # 'operator': expression is always 'boolean_value'
		/w14311 # 'variable': pointer truncation from 'type1' to 'type2'
		/w14545 # expression before comma evaluates to a function which is missing an argument list
		/w14546 # function call before comma missing argument list
		/w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
		/w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
		/w14555 # expression has no effect; expected expression with side-effect
		/w14619 # pragma warning: there is no warning number 'number'
		/w14640 # Enable warning on thread un-safe static member initialization
		/w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
		/w14905 # wide string literal cast to 'LPSTR'
		/w14906 # string literal cast to 'LPWSTR'
		/w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
	)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} PRIVATE "../source")

add_test( NAME ${PROJECT_NAME}
          COMMAND ${PROJECT_NAME}
)