function(set_up_warnings TARGET)
if(NOT MSVC)
	target_compile_options(${TARGET} PRIVATE 
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
	target_compile_options(${TARGET} PRIVATE 
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
endfunction(set_up_warnings)