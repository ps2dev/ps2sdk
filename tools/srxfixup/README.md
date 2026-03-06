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

## Zero-.text symbol check

When processing a relocatable ELF (`ET_REL`), `srxfixup` checks for function
or no-type symbols in the `.text` section whose `st_value` is exactly 0.  Such
symbols indicate that real code was placed first in `.text` by the linker.
If an IOP export table has a zeroed or uninitialized entry, the dispatcher
will issue a `jal 0x0` — silently jumping into whatever function sits at
`.text` offset 0 instead of faulting.  The symbols `_start` and `_ftext` are
excluded because they are entry points / linker labels that are never
dispatched through an export table.

If any offending symbol is found, `srxfixup` prints an error like:

```
ERROR: foo.irx: symbol 'bar' (sym#3) is in .text with value 0 -- if this module has an export table, put exports.o first in IOP_OBJS and move _retonly after DECLARE_EXPORT_TABLE in exports.tab; otherwise use --allow-zero-text.
```

and exits with a non-zero status so the build fails.

For modules **with** an export table, the fix is to ensure `exports.o` is
first in `IOP_OBJS` and that the `_retonly` definition appears after
`DECLARE_EXPORT_TABLE` in `exports.tab`.  This places the export-table struct
(`STT_OBJECT`) at `.text` offset 0 instead of function code.

For modules **without** an export table, the `jal 0x0` hazard does not apply.
Use `--allow-zero-text` to suppress the check.  The IOP `Rules.make`
automatically passes this flag when `exports.o` is not present in `IOP_OBJS`.
