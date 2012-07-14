#--------------------------------------------------------------------------------------------------------------------------------------------
#    Copyright (c) 2011, Eugene Mamin <TheDZhon@gmail.com>
#    All rights reserved.
#
#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions are met:
#        * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#        * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#        * Neither the name of the Prefix Increment nor the
#        names of its contributors may be used to endorse or promote products
#        derived from this software without specific prior written permission.
#
#    THIS SOFTWARE IS PROVIDED BY Eugene Mamin <TheDZhon@gmail.com> ''AS IS'' AND ANY
#    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#    DISCLAIMED. IN NO EVENT SHALL Eugene Mamin <TheDZhon@gmail.com> BE LIABLE FOR ANY
#    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

macro (win32_tune_libs_names name)
	IF (${name}_LIBRARY_RELEASE AND ${name}_LIBRARY_DEBUG)
		IF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPES)
			SET (${name}_LIBRARIES optimized ${${name}_LIBRARY_RELEASE} debug ${${name}_LIBRARY_DEBUG})
		ELSE (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPES)
			SET (${name}_LIBRARIES  ${${name}_LIBRARY_RELEASE})
			SET (_WIN32_ADDITIONAL_MESS 
				 "Warning: your generator doesn't support separate debug and release libs")
		ENDIF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPES)
	ENDIF (${name}_LIBRARY_RELEASE AND ${name}_LIBRARY_DEBUG)
	IF (${name}_LIBRARY_RELEASE AND NOT ${name}_LIBRARY_DEBUG)
		SET (${name}_LIBRARIES ${${name}_LIBRARY_RELEASE})
		SET (_WIN32_ADDITIONAL_MESS 
			 "Warning: debug version of "${name}" not found")
	ENDIF (${name}_LIBRARY_RELEASE AND NOT ${name}_LIBRARY_DEBUG)
	IF (NOT ${name}_LIBRARY_RELEASE AND ${name}_LIBRARY_DEBUG)
		SET (${name}_LIBRARIES ${${name}_LIBRARY_DEBUG})
		SET (_WIN32_ADDITIONAL_MESS 
			 "Warning: release version of "${name}" not found")	
	ENDIF (NOT ${name}_LIBRARY_RELEASE AND ${name}_LIBRARY_DEBUG)
endmacro(win32_tune_libs_names)