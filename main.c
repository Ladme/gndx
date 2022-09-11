// Released under MIT License.
// Copyright (c) 2022 Ladislav Bartos

// version 2022/09/11

#include <unistd.h>
#include <groan.h>

/*! @brief Splits selection query (NAME =/: SELECTION) into NAME and SELECTION. Returns zero if successful, else returns 1. */
int read_selection(const char *raw, char **name, char **query)
{
    char *copied = strdup(raw);

    char **split = NULL;
    size_t n_items = strsplit(copied, &split, "=:");

    if (n_items != 2) {
        free(split);
        free(copied);
        return 1;
    }

    *name = strdup(split[0]);
    *query = strdup(split[1]);

    strstrip(*name);
    strstrip(*query);

    free(split);
    free(copied);

    return 0;
}

/*
 * Parses command line arguments.
 * Returns zero, if parsing has been successful. Else returns non-zero.
 */
int get_arguments(
        int argc, 
        char **argv,
        char **gro_file,
        char **ndx_file,
        char **output_file,
        char **selection,
        char **name) 
{
    int gro_specified = 0, selection_specified = 0;

    int opt = 0;
    while((opt = getopt(argc, argv, "c:n:o:s:h")) != -1) {
        switch (opt) {
        // help
        case 'h':
            return 1;
        // gro file to read
        case 'c':
            *gro_file = optarg;
            gro_specified = 1;
            break;
        // ndx file to read
        case 'n':
            *ndx_file = optarg;
            break;
        // output file name
        case 'o':
            *output_file = optarg;
            break;
        // selected atoms
        case 's':
            if (read_selection(optarg, name, selection) != 0) {
                fprintf(stderr, "Could not understand selection query '%s'\n", optarg);
                return 1;
            }
            selection_specified = 1;
            break;
        default:
            //fprintf(stderr, "Unknown command line option: %c.\n", opt);
            return 1;
        }
    }

    if (!gro_specified || !selection_specified) {
        fprintf(stderr, "Selection and gro file must always be supplied.\n");
        return 1;
    }
    return 0;
}

void print_usage(const char *program_name)
{
    printf("Usage: %s -c GRO_FILE -s SELECTION [OPTION]...\n", program_name);
    printf("\nOPTIONS\n");
    printf("-h               print this message and exit\n");
    printf("-c STRING        gro file to read\n");
    printf("-s STRING        selection of atoms to create an ndx group for (e.g. \"Membrane = resname POPC\")\n");
    printf("-o STRING        output ndx file (optional)\n");
    printf("-n STRING        input ndx file (optional)\n");
    printf("\n");
}

void write_ndx_group(FILE *stream, const char *name, const atom_selection_t *selection)
{
    fprintf(stream, "[ %s ]\n", name);
    for (size_t i = 0; i < selection->n_atoms; ++i) {
        fprintf(stream, "%4ld ", selection->atoms[i]->gmx_atom_number);

        if ((i + 1) % 15 == 0 || i + 1 == selection->n_atoms) fprintf(stream, "\n");
    }
}

int main(int argc, char **argv)
{
    // get arguments
    char *gro_file = NULL;
    char *ndx_file = NULL;
    char *output_file = NULL;
    char *selected = NULL;
    char *name = NULL;

    if (get_arguments(argc, argv, &gro_file, &ndx_file, &output_file, &selected, &name) != 0) {
        print_usage(argv[0]);
        return 1;
    }
    
    // read gro file
    system_t *system = load_gro(gro_file);
    if (system == NULL) {
        free(selected);
        free(name);
        return 1;
    }

    // if input ndx file is provided, try reading it
    dict_t *ndx_groups = NULL;
    if (ndx_file != NULL) {
        ndx_groups = read_ndx(ndx_file, system);
        if (ndx_groups == NULL) {
            fprintf(stderr, "Could not read input ndx file %s\n", ndx_file);
            free(system);
            free(selected);
            free(name);
            return 1;
        }
    }

    // select all atoms
    atom_selection_t *all = select_system(system);

    // select selected atoms
    atom_selection_t *selection = smart_select(all, selected, ndx_groups);
    if (selection == NULL) {
        fprintf(stderr, "Could not understand the selection query '%s'\n", selected);

        dict_destroy(ndx_groups);
        free(system);
        free(all);
        free(selected);
        free(name);
        return 1;
    }

    // if the number of selected atoms is 0, print warning but continue
    if (selection->n_atoms == 0) {
        fprintf(stderr, "Warning. Selection query '%s' corresponds to no atoms.\n", selected);
    }

    // open the output file
    FILE *output = NULL;
    FILE *test = NULL;
    if (output_file == NULL) output = stdout;
    // if the file exists, append
    else if ((test = fopen(output_file, "r")) != NULL) {
        fclose(test);
        output = fopen(output_file, "a");
    } else {
        output = fopen(output_file, "w");
    }

    if (output == NULL) {
        fprintf(stderr, "The output ndx file could not be opened.\n");
        dict_destroy(ndx_groups);
        free(system);
        free(all);
        free(selection);
        free(selected);
        free(name);
        return 1;
    }

    // write the selection into an ndx file
    write_ndx_group(output, name, selection);

    dict_destroy(ndx_groups);
    free(system);
    free(all);
    free(selection);
    free(selected);
    free(name);

    if (output != stdout) fclose(output);

    return 0;
}