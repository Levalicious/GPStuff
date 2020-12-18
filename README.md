# GPStuff
Some experiments with genetic programming. Currently tinkering with heuristics for residue numeral systems.

## Contents:
vm.h & vm.c - Instructions are defined in vm.h, and code execution is done in vm.c
rns.h & rns.c - Completely unrelated to the rest of the library, but is used for the symbolic regression I'm currently using this library for
main.c - Mostly just a mess. Contains a bunch of utility functions for creating/randomizing/mutating/breeding/evaluating the fitness of candidate programs.

At some point most of the mess in main.c is going to get refactored out and into other files, and rns.h & rns.c will disappear.

## To Do:

Parallelization

Two approaches:
Parallelize the fitness evaluation loop

Advantages:
- Easy to do. Possibly 1 line of OpenMP directives

Disadvantages:
- Low benefits, especially for problems with low trial numbers
    - Switching between a main thread to cross-breed and the others to evaluate fitness ends up eliminating most of the speedup

Parallelize the entire main function

Advantages:
- Potentially faster

Disadvantages:
- More work to set up cross-'island' program movement
- More work to keep program deterministic
    - This parallelization approach + determinism might eliminate any speed advantages this approach would have over the other
    
    
