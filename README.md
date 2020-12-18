# GPStuff
Some experiments with genetic programming. Currently tinkering with heuristics for residue numeral systems.

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
    - This parallelization approach + determinism might eliminate any speed advantages
