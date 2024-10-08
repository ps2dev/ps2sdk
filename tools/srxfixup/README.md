# srxfixup

This tool mainly handles generation of IRX and ERX files, mainly used as
relocatable executables or libraries.  
This tool performs the following tasks on relocatable ELF files in order to
ensure that loadcore can load it as a relocatable file:  

* Sets the ELF header `e_type` to `0xFF80`, `0xFF81`, or `0xFF91`, depending
on the features and architecture used  

* Creates the `.iopmod` or `.eemod` section and first program header,
containing metadata about the file, such as name, version, and section sizes/offsets  

* Rebuilds relocations  

## Aliases

This tool provides the following:

* `iopfixup`
* `irx-strip`
* `eefixup`
* `erx-strip`

## Usage

To see usage and possible command line arguments that can be used with the
program, run it without arguments.
