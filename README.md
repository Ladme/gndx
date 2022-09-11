# gndx: Creating NDX Groups For Gromacs Simulations

Creates an ndx group for a selection of atoms specified using the [groan selection language](https://github.com/Ladme/groan#groan-selection-language).

## Dependencies

`gndx` requires you to have groan library installed. You can get groan from [here](https://github.com/Ladme/groan). See also the [installation instructions](https://github.com/Ladme/groan#installing) for groan.

## Installation

1) Run `make groan=PATH_TO_GROAN` to create a binary file `gndx` that you can place wherever you want. `PATH_TO_GROAN` is a path to the directory containing groan library (containing `groan.h` and `libgroan.a`).
2) (Optional) Run `make install` to copy the the binary file `gndx` into `${HOME}/.local/bin`.

## Options

```
Usage: gndx -c GRO_FILE -s SELECTION [OPTION]...

OPTIONS
-h               print this message and exit
-c STRING        gro file to read
-s STRING        selection of atoms to create an ndx group for (e.g. "Membrane = resname POPC")
-o STRING        output ndx file (optional)
-n STRING        input ndx file (optional)
```

Note that the flag `-s` expects an argument in the following format: `NDX GROUP NAME = SELECTION QUERY` or `NDX GROUP NAME : SELECTION QUERY`. Note that the whitespaces between `=` or `:` and the rest of the query are optional. `NDX GROUP NAME` specifies the name of the new ndx group and it can be any string of a maximal length of 99 characters. `SELECTION QUERY` is a selection of atoms specified using the [groan selection language](https://github.com/Ladme/groan#groan-selection-language).

Note that the option `-o` is optional. If it is not supplied, the generated ndx group is printed into standard output (usually the terminal). Note that if the specified output file matches the path to any existing file, the newly created ndx group is_appended_ to the end of the file. In case the file does not exist, it is created and the ndx group is written into it.

The input (`-n`) and output (`-o`) ndx file can be the same file. In that case, the new ndx group is added to the end of the original ndx file and the original ndx groups are not modified in any way.

## Examples

```
gndx -c md.gro -s "Leu_CA = name CA and resname LEU"
```

The program will read `md.gro` and select all atoms with the name `CA` corresponding to residues named `LEU`. The program will then create an ndx group named `Leu_CA` containing atom numbers of the selected atoms. The ndx group will be printed into terminal. In case no atoms have been selected, the program will inform the user about this fact but it will still create the (empty) ndx group.

```
gndx -c md.gro -s "Leu_CA = name CA and resname LEU" -n index.ndx -o index.ndx
```

The same as above but the newly created ndx group `Leu_CA` will be written into `index.ndx`. Other ndx groups in the `index.ndx` file will not be modified.

```
gndx -c md.gro -s "Leu_BB = Backbone and resname LEU" -n index.ndx -o group.ndx
```

The program will create an ndx group `Leu_BB` containing atoms corresponding to residues named `LEU` while also being a part of ndx group `Backbone`. The original ndx groups (including `Backbone`, if it exists) will be read from `index.ndx`, but the newly created ndx group will be written into a file `group.ndx`.

```
gndx -c md.gro -s "Leu_BB = Backbone and resname LEU" -o index.ndx
```

This command will result in an error because no input ndx file has been supplied (no `-n` flag) and thus the meaning of `Backbone` cannot be deciphered. 


## Limitations

When appending to a file, `gndx` expects the file to end with a newline character.

Only tested on Linux. Probably will not work on anything that is not UNIX-like.
