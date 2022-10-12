# Add executable called "mtfind.exe" that is built from the source files
# "mtfind.cpp", "noregex.cpp" and "regex.cpp". The extensions are automatically found.
add_executable (mtfind.exe mtfind.cpp noregex.cpp regex.cpp)
